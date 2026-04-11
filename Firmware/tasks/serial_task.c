#include "serial_task.h"

#include "globals.h"
#include "tcode_build_info.h"
#include "tcode_protocol.h"
#include "thermo_control_task.h"
#include "tools.h"
#include "semphr.h"
#include "pico/error.h"
#include "pico/stdio.h"
#include "hardware/timer.h"
#include "hardware/watchdog.h"
#include "hmi_uart.h"
#include "pindefs.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define SERIAL_LINE_CAP 256
#define SERIAL_REPLY_BUF_SIZE 512

/** Which interface supplied the current line */
typedef enum {
  SERIAL_REPLY_USB = 0,
  SERIAL_REPLY_HMI,
} serial_reply_to_t;

static SemaphoreHandle_t hmi_uart_mutex;
static volatile bool serial_hmi_ready;

static void hmi_uart_tx_locked_write(const uint8_t *data, size_t len) {
  if (!serial_hmi_ready || len == 0)
    return;
  if (hmi_uart_mutex &&
      xSemaphoreTake(hmi_uart_mutex, portMAX_DELAY) != pdTRUE)
    return;
  hmi_uart_write(data, len);
  if (hmi_uart_mutex)
    xSemaphoreGive(hmi_uart_mutex);
}

static void serial_write_reply(serial_reply_to_t to, const char *s, size_t len) {
  if (len == 0)
    return;
  if (to == SERIAL_REPLY_USB) {
    for (size_t i = 0; i < len; i++)
      putchar((unsigned char)s[i]);
    fflush(stdout);
  } else {
    hmi_uart_tx_locked_write((const uint8_t *)s, len);
  }
}

static void serial_reply_vprintf(serial_reply_to_t to, const char *fmt,
                                 va_list ap) {
  char buf[SERIAL_REPLY_BUF_SIZE];
  va_list ap2;
  va_copy(ap2, ap);
  int n = vsnprintf(buf, sizeof buf, fmt, ap2);
  va_end(ap2);
  if (n < 0)
    return;
  size_t len = (size_t)n;
  if (len >= sizeof buf)
    len = sizeof buf - 1u;
  serial_write_reply(to, buf, len);
}

static void serial_reply_printf(serial_reply_to_t to, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  serial_reply_vprintf(to, fmt, ap);
  va_end(ap);
}

// ------------------------
// TCode command processing
// ------------------------

static bool is_unsigned_int_token(const char *s) {
  if (!s || !*s)
    return false;
  for (const char *p = s; *p; ++p) {
    if (*p < '0' || *p > '9')
      return false;
  }
  return true;
}

static void process_tcode_line(char *line, serial_reply_to_t reply_to) {
  int line_number = 0;

  tcode_parsed_line_t parsed;
  tcode_status_t st = tcode_parse_inplace(line, &parsed);
  if (st != TCODE_OK) {
    if (st == TCODE_ERR_CHECKSUM_MISMATCH) {
      serial_reply_printf(reply_to, "ERROR: Wrong checksum! (got %02X, expected %02X)\n",
             parsed.calculated_checksum, parsed.given_checksum);
    } else if (st != TCODE_ERR_EMPTY) {
      serial_reply_printf(reply_to, "ERROR: Parse error (%s)\n", tcode_status_str(st));
    }
    return;
  }

  char **segments = parsed.tokens;
  int segment_count = (int)parsed.token_count;
  int cur_segment = 0;

  // Line number (optional, N123)
  if (segment_count > 0 && segments[cur_segment][0] == 'N') {
    char *ptr = segments[0] + 1;
    if (*ptr)
      line_number = atoi(ptr);
    (void)line_number;
    cur_segment++;
  }

  // Setpoint commands: T15/H55 (implicit zone) or Z0 T15/Z0 H55 (explicit zone)
  if (segment_count > 0) {
    const char *cmd = segments[cur_segment];
    if (cmd && (cmd[0] == 'T' || cmd[0] == 'H' || cmd[0] == 'Z')) {
      int zone = 0;
      const char *th = NULL;

      if (cmd[0] == 'Z') {
        const char *zone_str = cmd[1] ? (cmd + 1) : NULL;
        if (!zone_str) {
          serial_reply_printf(reply_to, "Error: expected Z0 T15\n");
        } else if (!is_unsigned_int_token(zone_str)) {
          serial_reply_printf(reply_to, "Error: bad zone\n");
        } else if (cur_segment + 1 >= segment_count) {
          serial_reply_printf(reply_to, "Error: expected T/H after Z\n");
        } else {
          zone = atoi(zone_str);
          th = segments[cur_segment + 1];
        }
      } else {
        th = cmd;
      }

      if (th) {
        if (!(th[0] == 'T' || th[0] == 'H') || th[1] == '\0') {
          serial_reply_printf(reply_to, "Error: bad setpoint\n");
        } else if (zone != 0) {
          serial_reply_printf(reply_to, "Error: zone not supported\n");
        } else {
          int value = atoi(th + 1);
          if (th[0] == 'T') {
            if (value < -45 || value > 90)
              serial_reply_printf(reply_to, "Error: temp out of range\n");
            else {
              current_temperature_setpoint = (float)value;
              chamber_request_arm_idle();
            }
          } else {
            if (value < 0 || value > 100)
              serial_reply_printf(reply_to, "Error: humidity out of range\n");
            else
              current_humidity_setpoint = (float)value;
          }
        }
      }
    }
  }

  // M (machine) command
  if (segment_count > 0 && segments[cur_segment][0] == 'M') {
    const char *marg = NULL;
    if (segments[cur_segment][1] != '\0') {
      marg = segments[cur_segment] + 1;
    } else if (cur_segment + 1 < segment_count) {
      marg = segments[cur_segment + 1];
    }
    if (marg) {
      if (!is_unsigned_int_token(marg)) {
        serial_reply_printf(reply_to, "Error: bad M\n");
        return;
      }

      int mcode = atoi(marg); // Convert the string to an integer
      if (mcode == 0) {
        chamber_request_standby();
        return;
      }else if (mcode == 2) {
        // Hard stop: reboot the controller.
        // watchdog_reboot() triggers the watchdog-based reset path.
        watchdog_reboot(0, 0, 1);

        // Should never return;
        return;
      }

      serial_reply_printf(reply_to, "Error: unsupported M%d\n", mcode);
      return;
    } else {
      serial_reply_printf(reply_to, "Error: Missing M command argument\n");
      return;
    }
  }

  // Q (query) command
  if (segment_count > 0 && segments[cur_segment][0] == 'Q') {
    const char *qarg = segments[cur_segment] + 1;

    if (!qarg || *qarg == '\0' || !is_unsigned_int_token(qarg)) {
      serial_reply_printf(reply_to, "Error: bad Q\n");
      return;
    }

    if (strcmp(qarg, "0") == 0) {
      serial_reply_printf(reply_to, "data: TEMP=%.1f RH=%.1f HEAT=%s COOL=%s STATE=%s SET_TEMP=%.1f "
             "SET_RH=%.1f FAULT=%s DOOR=%s POWER=%.1f\n",
             sht35_temperature_c, sht35_humidity,
             heater_on ? "true" : "false",
             compressor_on ? "true" : "false",
             chamber_state_string((chamber_state_t)chamber_fsm_state),
             current_temperature_setpoint, current_humidity_setpoint,
             fault_code_string(FAULT), door_open ? "true" : "false",
             current_power);
    } else if (strcmp(qarg, "1") == 0) {
      const char *q1_arg = NULL;
      if (cur_segment + 1 < segment_count)
        q1_arg = segments[cur_segment + 1];

      if (q1_arg && strcmp(q1_arg, "BUILD") == 0) {
        serial_reply_printf(reply_to, "data: BUILD=%s\n", TCODE_BUILD_GIT_DESCRIBE);
      } else if (q1_arg && strcmp(q1_arg, "BUILDER") == 0) {
        serial_reply_printf(reply_to, "data: BUILDER=%s\n", TCODE_BUILD_BUILDER);
      } else if (q1_arg && strcmp(q1_arg, "BUILD_DATE") == 0) {
        serial_reply_printf(reply_to, "data: BUILD_DATE=%s\n", TCODE_BUILD_DATE_UNIX);
      } else if (q1_arg && strcmp(q1_arg, "CT0_AMPS") == 0) {
        serial_reply_printf(reply_to, "data: CT0_AMPS=%.2f\n", ct0_amps);
      } else if (q1_arg && strcmp(q1_arg, "CT1_AMPS") == 0) {
        serial_reply_printf(reply_to, "data: CT1_AMPS=%.2f\n", ct1_amps);
      } else if (q1_arg && strcmp(q1_arg, "CT2_AMPS") == 0) {
        serial_reply_printf(reply_to, "data: CT2_AMPS=%.2f\n", ct2_amps);
      } else if (q1_arg && strcmp(q1_arg, "CT3_AMPS") == 0) {
        serial_reply_printf(reply_to, "data: CT3_AMPS=%.2f\n", ct3_amps);
      } else if (q1_arg && strcmp(q1_arg, "TDR0_TEMPERATURE_C") == 0) {
        serial_reply_printf(reply_to, "data: TDR0_TEMPERATURE_C=%.2f\n", tdr0_temperature_c);
      } else if (q1_arg && strcmp(q1_arg, "TDR1_TEMPERATURE_C") == 0) {
        serial_reply_printf(reply_to, "data: TDR1_TEMPERATURE_C=%.2f\n", tdr1_temperature_c);
      } else if (q1_arg && strcmp(q1_arg, "TDR2_TEMPERATURE_C") == 0) {
        serial_reply_printf(reply_to, "data: TDR2_TEMPERATURE_C=%.2f\n", tdr2_temperature_c);
      } else if (q1_arg && strcmp(q1_arg, "TDR3_TEMPERATURE_C") == 0) {
        serial_reply_printf(reply_to, "data: TDR3_TEMPERATURE_C=%.2f\n", tdr3_temperature_c);
      } else if (q1_arg && strcmp(q1_arg, "STATE") == 0) {
        serial_reply_printf(reply_to, "data: STATE=%s\n",
               chamber_state_string((chamber_state_t)chamber_fsm_state));
      } else if (q1_arg && strcmp(q1_arg, "FAULT") == 0) {
        serial_reply_printf(reply_to, "data: FAULT=%s\n", fault_code_string(FAULT));
      } else if (q1_arg && strcmp(q1_arg, "COMPRESSOR_ON_TIME") == 0) {
        serial_reply_printf(reply_to, "data: COMPRESSOR_ON_TIME=%u\n",
               (unsigned int)thermo_control_get_compressor_on_time());
      } else if (q1_arg && strcmp(q1_arg, "COMPRESSOR_OFF_TIME") == 0) {
        serial_reply_printf(reply_to, "data: COMPRESSOR_OFF_TIME=%u\n",
               (unsigned int)thermo_control_get_compressor_off_time());
      } else if (q1_arg && strcmp(q1_arg, "SHT35_TEMPERATURE_C") == 0) {
        serial_reply_printf(reply_to, "data: SHT35_TEMPERATURE_C=%.2f\n", sht35_temperature_c);
      } else if (q1_arg && strcmp(q1_arg, "SHT35_HUMIDITY") == 0) {
        serial_reply_printf(reply_to, "data: SHT35_HUMIDITY=%.2f\n", sht35_humidity);
      } else if (q1_arg && strcmp(q1_arg, "I2C_SCAN") == 0) {
        char buf[50];
        build_i2c_scan_string(buf, sizeof(buf));
        serial_reply_printf(reply_to, "data: I2C_SCAN=%s\n", buf);
      } else if (q1_arg && strcmp(q1_arg, "FREERTOS_HEAP_FREE") == 0) {
        serial_reply_printf(reply_to, "data: FREERTOS_HEAP_FREE=%u\n", (unsigned int)xPortGetFreeHeapSize());
      } else if (q1_arg && strcmp(q1_arg, "FREERTOS_HEAP_MIN") == 0) {
        // Returns the minimum ever free heap size in bytes
        serial_reply_printf(reply_to, "data: FREERTOS_HEAP_MIN=%u\n", (unsigned int)xPortGetMinimumEverFreeHeapSize());
      // } else if (q1_arg && strcmp(q1_arg, "FREERTOS_TASKS_RUNTIME") == 0) {
      //   char stats[512];
      //   vTaskGetRunTimeStats(stats);
      //   serial_reply_printf(reply_to, "data: FREERTOS_TASKS_RUNTIME_START\n%sdata: FREERTOS_TASKS_RUNTIME_END\n", stats);
      // } else if (q1_arg && strcmp(q1_arg, "FREERTOS_TASKS_LIST") == 0) {
      //   // Show an overview of task state/stacks, etc.
      //   char list[512];
      //   vTaskList(list);
      //   serial_reply_printf(reply_to, "data: FREERTOS_TASKS_LIST_START\n%sdata: FREERTOS_TASKS_LIST_END\n", list);
      } else {
        serial_reply_printf(reply_to, "error:UNKNOWN_KEY %s\n", q1_arg ? q1_arg : "(missing)");
      }
    } else {
      serial_reply_printf(reply_to, "Error: %s not a valid query command\n", qarg ? qarg : "(missing)");
    }
  }
}

static void feed_line_char(const serial_task_config_t *cfg, int c,
                           serial_reply_to_t reply_to, char *line_buf,
                           int *line_idx) {
  if (cfg && cfg->enable_echo && *(cfg->enable_echo)) {
    if (reply_to == SERIAL_REPLY_USB) {
      putchar(c);
      fflush(stdout);
    } else {
      uint8_t b = (uint8_t)c;
      hmi_uart_tx_locked_write(&b, 1u);
    }
  }

  if (c == '\n' || c == '\r') {
    if (*line_idx > 0) {
      line_buf[*line_idx] = '\0';
      process_tcode_line(line_buf, reply_to);
      serial_reply_printf(reply_to, "ok\n");
      *line_idx = 0;
    }
  } else if (*line_idx < SERIAL_LINE_CAP - 1) {
    line_buf[(*line_idx)++] = (char)c;
  }
}

// -----------
// Serial task
// -----------

static void serial_task(void *pvParameters) {
  const serial_task_config_t *cfg = (const serial_task_config_t *)pvParameters;

  char line_usb[SERIAL_LINE_CAP];
  char line_hmi[SERIAL_LINE_CAP];
  int idx_usb = 0;
  int idx_hmi = 0;

  if (hmi_uart_mutex == NULL) {
    hmi_uart_mutex = xSemaphoreCreateMutex();
  }

  hmi_uart_init();
  serial_hmi_ready = true;

  while (true) {
    bool work = false;

    int c = getchar_timeout_us(0);
    if (c != PICO_ERROR_TIMEOUT) {
      feed_line_char(cfg, c, SERIAL_REPLY_USB, line_usb, &idx_usb);
      work = true;
    }

    for (;;) {
      int ch = hmi_uart_try_getc();
      if (ch < 0)
        break;
      feed_line_char(cfg, ch, SERIAL_REPLY_HMI, line_hmi, &idx_hmi);
      work = true;
    }

    /* PIO RX FIFO is only 4 deep at 115200 (~11 chars/ms). A 1 ms sleep here
       overflowed the FIFO and corrupted lines (e.g. stray '*' -> CHECKSUM_FORMAT). */
    if (!work)
      busy_wait_us(100);
  }
}

void serial_hmi_puts(const char *s) {
  if (!s)
    return;
  size_t n = strlen(s);
  if (n == 0)
    return;
  hmi_uart_tx_locked_write((const uint8_t *)s, n);
}

BaseType_t serial_task_create(const serial_task_config_t *cfg,
                              UBaseType_t priority, TaskHandle_t *out_handle) {
  return xTaskCreate(serial_task, "serial", 1024, (void *)cfg, priority,
                     out_handle);
}

