#include <Adafruit_GFX.h>
#include <LovyanGFX.hpp>
#include <PCA9557.h>
#include <SPI.h>
#include <lgfx/v1/platforms/esp32s3/Bus_RGB.hpp>
#include <lgfx/v1/platforms/esp32s3/Panel_RGB.hpp>
#include <lvgl.h>

// Define TOUCH_GT911 before including touch.h
#define TOUCH_GT911
#define TOUCH_GT911_SCL 20
#define TOUCH_GT911_SDA 19
#define TOUCH_GT911_INT -1
#define TOUCH_GT911_RST -1
#define TOUCH_GT911_ROTATION ROTATION_NORMAL
#define TOUCH_MAP_X1 800
#define TOUCH_MAP_X2 0
#define TOUCH_MAP_Y1 480
#define TOUCH_MAP_Y2 0

// Touch object will be defined in touch.h after lcd is defined

class LGFX : public lgfx::LGFX_Device {
public:
  lgfx::Bus_RGB _bus_instance;
  lgfx::Panel_RGB _panel_instance;

  LGFX(void) {

    {
      auto cfg = _bus_instance.config();
      cfg.panel = &_panel_instance;

      cfg.pin_d0 = GPIO_NUM_15; // B0
      cfg.pin_d1 = GPIO_NUM_7;  // B1
      cfg.pin_d2 = GPIO_NUM_6;  // B2
      cfg.pin_d3 = GPIO_NUM_5;  // B3
      cfg.pin_d4 = GPIO_NUM_4;  // B4

      cfg.pin_d5 = GPIO_NUM_9;  // G0
      cfg.pin_d6 = GPIO_NUM_46; // G1
      cfg.pin_d7 = GPIO_NUM_3;  // G2
      cfg.pin_d8 = GPIO_NUM_8;  // G3
      cfg.pin_d9 = GPIO_NUM_16; // G4
      cfg.pin_d10 = GPIO_NUM_1; // G5

      cfg.pin_d11 = GPIO_NUM_14; // R0
      cfg.pin_d12 = GPIO_NUM_21; // R1
      cfg.pin_d13 = GPIO_NUM_47; // R2
      cfg.pin_d14 = GPIO_NUM_48; // R3
      cfg.pin_d15 = GPIO_NUM_45; // R4

      cfg.pin_henable = GPIO_NUM_41;
      cfg.pin_vsync = GPIO_NUM_40;
      cfg.pin_hsync = GPIO_NUM_39;
      cfg.pin_pclk = GPIO_NUM_0;
      cfg.freq_write = 15000000;

      cfg.hsync_polarity = 0;
      cfg.hsync_front_porch = 40;
      cfg.hsync_pulse_width = 48;
      cfg.hsync_back_porch = 40;

      cfg.vsync_polarity = 0;
      cfg.vsync_front_porch = 1;
      cfg.vsync_pulse_width = 31;
      cfg.vsync_back_porch = 13;

      cfg.pclk_active_neg = 1;
      cfg.de_idle_high = 0;
      cfg.pclk_idle_high = 0;

      _bus_instance.config(cfg);
    }
    {
      auto cfg = _panel_instance.config();
      cfg.memory_width = 800;
      cfg.memory_height = 480;
      cfg.panel_width = 800;
      cfg.panel_height = 480;
      cfg.offset_x = 0;
      cfg.offset_y = 0;
      _panel_instance.config(cfg);
    }
    _panel_instance.setBus(&_bus_instance);
    setPanel(&_panel_instance);
  }
};

LGFX lcd;

// Include touch.h after lcd is defined (touch.h needs to reference lcd)
#include "touch.h"

// UI includes
#include "screens.h" // Need this to access objects.main directly
#include "ui.h"

// UI
#define TFT_BL 2
SPIClass &spi = SPI;

/* Change to your screen resolution */
static uint32_t screenWidth;
static uint32_t screenHeight;
static lv_disp_draw_buf_t draw_buf;
// static lv_color_t *disp_draw_buf;
static lv_color_t disp_draw_buf[800 * 480 / 15];
static lv_disp_drv_t disp_drv;

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area,
                   lv_color_t *color_p) {
  // Removed debug output for faster rendering

  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

#if (LV_COLOR_16_SWAP != 0)
  lcd.pushImageDMA(area->x1, area->y1, w, h, (lgfx::rgb565_t *)color_p);
#else
  lcd.pushImageDMA(area->x1, area->y1, w, h, (lgfx::rgb565_t *)color_p);
#endif

  lv_disp_flush_ready(disp);
}

void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
  static unsigned long last_touch_debug = 0;
  static unsigned long call_count = 0;
  unsigned long now = millis();

  call_count++;
  // Print every 100 calls to confirm callback is being invoked
  if (call_count % 100 == 0) {
    Serial.print("[TOUCH] Callback called #");
    Serial.println(call_count);
  }

  if (touch_has_signal()) {
    if (call_count % 100 == 0) {
      Serial.println("[TOUCH] touch_has_signal() returned true");
    }

    if (touch_touched()) {
      data->state = LV_INDEV_STATE_PR;

      /*Set the coordinates*/
      data->point.x = touch_last_x;
      data->point.y = touch_last_y;

      // Debug output (rate limited to avoid spam)
      if (now - last_touch_debug > 100) {
        Serial.print("[TOUCH] TOUCHED - x: ");
        Serial.print(data->point.x);
        Serial.print(", y: ");
        Serial.println(data->point.y);
        last_touch_debug = now;
      }
    } else if (touch_released()) {
      data->state = LV_INDEV_STATE_REL;
      Serial.println("[TOUCH] RELEASED");
    } else {
      data->state = LV_INDEV_STATE_REL;
    }
  } else {
    data->state = LV_INDEV_STATE_REL;
    // Only print once in a while to avoid spam
    static unsigned long last_no_signal = 0;
    if (now - last_no_signal > 5000) {
      Serial.println("[TOUCH] No signal (this is normal when not touching)");
      last_no_signal = now;
    }
  }
  delay(15);
}

// PCA9557 Out;

// C-compatible wrapper functions for Serial debugging
extern "C" {
void Serial_print(const char *str) { Serial.print(str); }
void Serial_println(const char *str) { Serial.println(str); }
void Serial_print_hex(uint32_t val) { Serial.print(val, HEX); }
void Serial_println_hex(uint32_t val) { Serial.println(val, HEX); }
void Serial_print_int(int val) { Serial.print(val); }
void Serial_println_int(int val) { Serial.println(val); }
}

void setup() {

  Serial.begin(9600);
  delay(500); // Wait for Serial to be ready
  Serial.println("[MAIN] Firmware Starting");
  // Out.reset();
  // Out.setMode(IO_OUTPUT);  //设置为输出模式

  // Out.setState(IO0, IO_LOW);
  // Out.setState(IO1, IO_LOW);
  // delay(20);
  // Out.setState(IO0, IO_HIGH);
  // delay(100);
  // Out.setMode(IO1, IO_INPUT);
  //  EEPROM.begin(1024);
  //  if (get_string(1, 0) == e_flag) {
  //  //如果第0位字符为“M”，则表示已储存WIFI信息
  //    Serial.println("已储存信息");
  //    pinMode(SD_CS, OUTPUT);      digitalWrite(SD_CS, HIGH);
  //    SPI.begin(SD_SCK, SD_MISO, SD_MOSI);
  //    SPI.setFrequency(1000000);
  //    SD.begin(SD_CS);
  //    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  //    audio.setVolume(15); // 0...21
  //    audio.connecttoFS(SD, "/123.mp3");
  //    while (1)
  //    {
  //      unsigned long currentMillis = millis();
  //      //检查是否到达时间间隔
  //      if (currentMillis - previousBlinkTime >= blinkInterval) {
  //      //如果时间间隔达到了
  //        audio.stopSong();
  //        break;
  //      }
  //      else if (currentMillis - previousBlinkTime <= 0)
  //      { // 如果millis时间溢出
  //        previousBlinkTime = currentMillis;
  //      }
  //      audio.loop();
  //    }
  //  }
  //  else {
  //    Serial.println("未储存有WIFI信息,将储存wifi信息");
  //    //保存信息
  //    set_string(1, 0, e_flag, 0); //录入"N"字符
  //    Serial.println("完成储存");
  //    Serial.println("即将重启");
  //    ESP.restart();
  //  }
  //  EEPROM.end();

  // Init Display
  lcd.begin();
  lcd.fillScreen(TFT_BLACK);
  lcd.setTextSize(2);
  delay(50);
  Serial.println("[MAIN] Display cleared");

  //    lcd.fillScreen(TFT_RED);
  //    delay(1000);
  //    lcd.fillScreen(TFT_GREEN);
  //    delay(1000);
  //    lcd.fillScreen(TFT_BLUE);
  //    delay(1000);
  //    lcd.fillScreen(TFT_BLACK);
  //    delay(1000);
  lv_init();

  // Init touch device (touch_init() will call Wire.begin())
  touch_init();

  screenWidth = lcd.width();
  screenHeight = lcd.height();

  lv_disp_draw_buf_init(&draw_buf, disp_draw_buf, NULL,
                        screenWidth * screenHeight / 15);

  /* Initialize the display */
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  /* Initialize the input device driver */
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);
#ifdef TFT_BL

  // digitalWrite(TFT_BL, HIGH);
  ledcSetup(1, 300, 8);
  ledcAttachPin(TFT_BL, 1);
  ledcWrite(1, 255); /* Screen brightness can be modified by adjusting this
                        parameter. (0-255) */
#endif

#ifdef TFT_BL
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, LOW);
  delay(500);
  digitalWrite(TFT_BL, HIGH);
#endif

  ui_init(); // 开机UI界面

  // Post-UI initialization fix: Ensure all screen children render on top
  // The issue: ui_init() calls loadScreen() which uses lv_scr_load_anim()
  // The animation can cause a white screen overlay. We fix this by:
  // 1. Getting the screen directly from objects.main
  // 2. Reloading it immediately without animation
  // 3. Ensuring all children are visible and on top
  extern objects_t objects;
  lv_obj_t *main_screen = objects.main;

  if (main_screen) {
    // Load screen immediately without animation (replaces the animated load)
    lv_scr_load(main_screen);

    // Process LVGL to ensure screen is active
    lv_timer_handler();

    // Move all children to foreground to ensure they render above background
    uint32_t child_cnt = lv_obj_get_child_cnt(main_screen);
    for (uint32_t i = 0; i < child_cnt; i++) {
      lv_obj_t *child = lv_obj_get_child(main_screen, i);
      if (child) {
        lv_obj_move_foreground(child);
        lv_obj_invalidate(child);
      }
    }

    // Invalidate entire screen to force redraw
    lv_obj_invalidate(main_screen);

    // Force immediate refresh
    lv_refr_now(lv_disp_get_default());
  }

  // Force initial render
  for (int i = 0; i < 3; i++) {
    lv_timer_handler();
    delay(10);
  }
}

void loop() {
  // Update UI
  ui_tick();
  lv_timer_handler(); /* let the GUI do its work */
  delay(10);
}
