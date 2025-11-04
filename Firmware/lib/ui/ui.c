#if defined(EEZ_FOR_LVGL)
#include <eez/core/vars.h>
#endif

#include "actions.h"
#include "images.h"
#include "screens.h"
#include "ui.h"
#include "vars.h"

#ifdef __cplusplus
#include <Arduino.h>
#define DBG_PRINT(x) Serial.print(x)
#define DBG_PRINTLN(x) Serial.println(x)
#define DBG_PRINT_HEX(x) Serial.print((uint32_t)x, HEX)
#define DBG_PRINTLN_HEX(x) Serial.println((uint32_t)x, HEX)
#define DBG_PRINT_INT(x) Serial.print(x)
#define DBG_PRINTLN_INT(x) Serial.println(x)
#else
// For C code, we'll need to use printf or declare Serial extern
extern void Serial_print(const char *);
extern void Serial_println(const char *);
extern void Serial_print_hex(uint32_t);
extern void Serial_println_hex(uint32_t);
extern void Serial_print_int(int);
extern void Serial_println_int(int);
#define DBG_PRINT(x) Serial_print(x)
#define DBG_PRINTLN(x) Serial_println(x)
#define DBG_PRINT_HEX(x) Serial_print_hex((uint32_t)x)
#define DBG_PRINTLN_HEX(x) Serial_println_hex((uint32_t)x)
#define DBG_PRINT_INT(x) Serial_print_int(x)
#define DBG_PRINTLN_INT(x) Serial_println_int(x)
#endif

#if defined(EEZ_FOR_LVGL)

void ui_init() {
  eez_flow_init(assets, sizeof(assets), (lv_obj_t **)&objects, sizeof(objects),
                images, sizeof(images), actions);
}

void ui_tick() {
  eez_flow_tick();
  tick_screen(g_currentScreen);
}

#else

#include <string.h>

static int16_t currentScreen = -1;

static lv_obj_t *getLvglObjectFromIndex(int32_t index) {
  if (index == -1) {
    return 0;
  }
  // Access the screen object directly from the struct
  // SCREEN_ID_MAIN = 1, so index = 0 after subtracting 1
  if (index == 0) {
    return objects.main;
  }
  return 0;
}

void loadScreen(enum ScreensEnum screenId) {
  DBG_PRINT("[UI] loadScreen() called with screenId = ");
  DBG_PRINTLN_INT(screenId);
  currentScreen = screenId - 1;
  DBG_PRINT("[UI] currentScreen = ");
  DBG_PRINTLN_INT(currentScreen);
  lv_obj_t *screen = getLvglObjectFromIndex(currentScreen);
  DBG_PRINT("[UI] screen pointer = ");
  DBG_PRINTLN_HEX(screen);
  if (screen) {
    DBG_PRINTLN("[UI] Loading screen directly");
    // Use lv_scr_load instead of animation to ensure immediate activation
    lv_scr_load(screen);
    // Force invalidate to ensure redraw
    lv_obj_invalidate(screen);
  } else {
    DBG_PRINTLN("[UI] ERROR: screen pointer is NULL!");
  }
}

void ui_init() {
  DBG_PRINTLN("[UI] ui_init() called");
  create_screens();
  DBG_PRINTLN("[UI] create_screens() completed");
  DBG_PRINT("[UI] objects.main = ");
  DBG_PRINTLN_HEX(objects.main);
  loadScreen(SCREEN_ID_MAIN);
  DBG_PRINTLN("[UI] loadScreen() completed");
}

void ui_tick() { tick_screen(currentScreen); }

#endif
