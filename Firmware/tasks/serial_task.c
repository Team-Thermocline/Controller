#include "serial_task.h"

#include "tcode_build_info.h"
#include "tcode_protocol.h"
#include "pico/error.h"
#include "pico/stdio.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ------------------------
// TCode command processing
// ------------------------

extern float current_temperature_setpoint;
extern float current_humidity_setpoint;
extern float current_temperature;
extern float current_humidity;
extern bool heater_on;
extern bool compressor_on;
extern int current_state;
extern int alarm_state;

static bool is_unsigned_int_token(const char *s) {
  if (!s || !*s)
    return false;
  for (const char *p = s; *p; ++p) {
    if (*p < '0' || *p > '9')
      return false;
  }
  return true;
}

static void process_tcode_line(char *line) {
  int line_number = 0;

  tcode_parsed_line_t parsed;
  tcode_status_t st = tcode_parse_inplace(line, &parsed);
  if (st != TCODE_OK) {
    if (st == TCODE_ERR_CHECKSUM_MISMATCH) {
      printf("ERROR: Wrong checksum! (got %02X, expected %02X)\n",
             parsed.calculated_checksum, parsed.given_checksum);
    } else if (st != TCODE_ERR_EMPTY) {
      printf("ERROR: Parse error (%s)\n", tcode_status_str(st));
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
          printf("Error: expected Z0 T15\n");
        } else if (!is_unsigned_int_token(zone_str)) {
          printf("Error: bad zone\n");
        } else if (cur_segment + 1 >= segment_count) {
          printf("Error: expected T/H after Z\n");
        } else {
          zone = atoi(zone_str);
          th = segments[cur_segment + 1];
        }
      } else {
        th = cmd;
      }

      if (th) {
        if (!(th[0] == 'T' || th[0] == 'H') || th[1] == '\0') {
          printf("Error: bad setpoint\n");
        } else if (zone != 0) {
          printf("Error: zone not supported\n");
        } else {
          int value = atoi(th + 1);
          if (th[0] == 'T') {
            if (value < -45 || value > 90)
              printf("Error: temp out of range\n");
            else
              current_temperature_setpoint = (float)value;
          } else {
            if (value < 0 || value > 100)
              printf("Error: humidity out of range\n");
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
      printf("Machine command: %s\n", marg);
    } else {
      printf("Error: Missing M command argument\n");
    }
  }

  // Q (query) command
  if (segment_count > 0 && segments[cur_segment][0] == 'Q') {
    const char *qarg = segments[cur_segment] + 1;

    if (!qarg || *qarg == '\0' || !is_unsigned_int_token(qarg)) {
      printf("Error: bad Q\n");
      return;
    }

    if (strcmp(qarg, "0") == 0) {
      const char *state_str = "UNKNOWN";
      switch (current_state) {
      case 0:
        state_str = "IDLE";
        break;
      case 1:
        state_str = "RUN";
        break;
      case 2:
        state_str = "STOP";
        break;
      case 3:
        state_str = "FAULT";
        break;
      }
      printf("data: TEMP=%.1f RH=%.1f HEAT=%s COOL=%s STATE=%s SET_TEMP=%.1f "
             "SET_RH=%.1f ALARM=%d\n",
             current_temperature, current_humidity,
             heater_on ? "true" : "false",
             compressor_on ? "true" : "false",
             state_str,
             current_temperature_setpoint, current_humidity_setpoint,
             alarm_state);
    } else if (strcmp(qarg, "1") == 0) {
      const char *q1_arg = NULL;
      if (cur_segment + 1 < segment_count)
        q1_arg = segments[cur_segment + 1];

      if (q1_arg && strcmp(q1_arg, "BUILD") == 0) {
        printf("data: BUILD=%s\n", TCODE_BUILD_GIT_DESCRIBE);
      } else if (q1_arg && strcmp(q1_arg, "BUILDER") == 0) {
        printf("data: BUILDER=%s\n", TCODE_BUILD_BUILDER);
      } else if (q1_arg && strcmp(q1_arg, "BUILD_DATE") == 0) {
        printf("data: BUILD_DATE=%s\n", TCODE_BUILD_DATE_UNIX);
      } else {
        printf("error:UNKNOWN_KEY %s\n", q1_arg ? q1_arg : "(missing)");
      }
    } else {
      printf("Error: %s not a valid query command\n", qarg ? qarg : "(missing)");
    }
  }
}

// -----------
// Serial task
// -----------

static void serial_task(void *pvParameters) {
  const serial_task_config_t *cfg = (const serial_task_config_t *)pvParameters;

  char line_buffer[256];
  int line_index = 0;

  while (true) {
    int c = getchar_timeout_us(0);

    if (c == PICO_ERROR_TIMEOUT) {
      vTaskDelay(pdMS_TO_TICKS(1));
      continue;
    }

    if (cfg && cfg->enable_echo && *(cfg->enable_echo)) {
      putchar(c);
      fflush(stdout);
    }

    if (c == '\n' || c == '\r') {
      if (line_index > 0) {
        line_buffer[line_index] = '\0';
        process_tcode_line(line_buffer);
        printf("ok\n");
        fflush(stdout);
        line_index = 0;
      }
    } else if (line_index < (int)sizeof(line_buffer) - 1) {
      line_buffer[line_index++] = (char)c;
    }
  }
}

BaseType_t serial_task_create(const serial_task_config_t *cfg,
                              UBaseType_t priority, TaskHandle_t *out_handle) {
  return xTaskCreate(serial_task, "serial", 1024, (void *)cfg, priority,
                     out_handle);
}

