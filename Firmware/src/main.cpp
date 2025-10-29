// Core
#include <Arduino.h>

#include <PicoDVI.h>
#include <Wire.h>
#include <lvgl.h>

DVIGFX16 display(DVI_RES_320x240p60, picodvi_dvi_cfg);

static const uint16_t screenWidth = 320;
static const uint16_t screenHeight = 240;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[screenWidth * screenHeight / 10];

// LVGL display flush callback
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area,
                   lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);
  display.drawRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
  lv_disp_flush_ready(disp);
}

void setup() {
  // Serial Configuration (USB emulator)
  Serial.begin(115200);

  while (!display.begin()) {
    Serial.println("Waiting for screen..");
    delay(1000);
  }

  display.setRotation(0); // Takes effect on next drawing command
  display.fillScreen(0x0000);

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
}

void loop() {
  // Run timer
  lv_timer_handler();
  delay(5);

  // delay(1000);
  // Serial.println("Waiting...");
  // display.fillScreen(0x0000);
  // delay(1000);
  // display.fillScreen(0xffff);
}