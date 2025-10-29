// Core
#include <Arduino.h>

#include <PicoDVI.h>
#include <Wire.h>
// #include <lvgl.h>

DVIGFX16 display(DVI_RES_320x240p60, picodvi_dvi_cfg);

static const uint16_t screenWidth = 320;
static const uint16_t screenHeight = 240;
// static lv_disp_draw_buf_t draw_buf;
// static lv_color_t buf1[screenWidth * screenHeight / 10];

void setup() {
  // Serial Configuration (USB emulator)
  Serial.begin(115200);

  while (!display.begin()) {
    Serial.println("Waiting for screen..");
    delay(1000);
  }

  display.setRotation(0); // Takes effect on next drawing command
  display.fillScreen(0x0000);

  Serial.println("Ready!");
}

void loop() {
  // Run timer
  // lv_timer_handler();
  delay(1000);
  Serial.println("Waiting...");
  display.fillScreen(0x0000);
  delay(1000);
  display.fillScreen(0xffff);
}