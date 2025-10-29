// Core
#include <Arduino.h>

#include "ui.h"
#include <PicoDVI.h>
#include <Wire.h>
#include <lvgl.h>

// Custom config using PIO1 instead of PIO0 to avoid conflict
static const struct dvi_serialiser_cfg custom_pio1_cfg = {
    .pio = pio1,
    .sm_tmds = {0, 1, 2},
    .pins_tmds = {10, 12, 14},
    .pins_clk = 8,
    .invert_diffpairs = true};

DVIGFX16 display(DVI_RES_320x240p60, custom_pio1_cfg);

static const uint16_t screenWidth = 320;
static const uint16_t screenHeight = 240;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[screenWidth * screenHeight / 10];

// Stub implementations for UI variables (need extern "C" for C linkage)
extern "C" {
const char *get_var_version() { return "1.0.0"; }
void set_var_version(const char *value) {}
double get_var_chamber_current() { return 25.0; }
void set_var_chamber_current(double value) {}
double get_var_chamber_set() { return 30.0; }
void set_var_chamber_set(double value) {}
const char *get_var_setpoint_ready() { return "Ready"; }
void set_var_setpoint_ready(const char *value) {}
}

// LVGL display flush callback
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area,
                   lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);
  display.drawRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
  lv_disp_flush_ready(disp);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Add delay to let framework finish claiming resources
  Serial.println("Waiting for framework to stabilize...");
  delay(2000);

  Serial.println("Initializing display...");

  if (!display.begin()) {
    // Use an infinite loop and LED or something to show error
    pinMode(LED_BUILTIN, OUTPUT);
    while (1) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(500);
      digitalWrite(LED_BUILTIN, LOW);
      delay(500);
    }
  }

  Serial.println("Display initialized!");

  display.setRotation(0); // Takes effect on next drawing command

  // Draw some test shapes to verify display works
  display.fillScreen(0x001F);                 // Blue background
  display.fillRect(50, 50, 220, 140, 0xF800); // Red rectangle
  display.fillCircle(160, 120, 40, 0x07E0);   // Green circle
  display.setTextSize(2);
  display.setTextColor(0xFFFF); // White text
  display.setCursor(60, 200);
  display.println("PIO1 Works!");

  delay(2000);                // Show test patterns for 2 seconds
  display.fillScreen(0x0000); // Black screen
  display.setTextSize(1);
  display.setCursor(80, 100);
  display.setTextColor(0xFFFF);
  display.println("LVGL Starting...");

  // Initialize LVGL
  lv_init();

  // Initialize display buffer
  lv_disp_draw_buf_init(&draw_buf, buf1, NULL, screenWidth * screenHeight / 10);

  // Initialize and register display driver
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  Serial.println("LVGL Ready!");

  // Initialize UI
  ui_init();
  Serial.println("UI Initialized!");
}

void loop() {
  // Run LVGL timer
  lv_timer_handler();

  // Run UI tick
  ui_tick();

  delay(5);
}