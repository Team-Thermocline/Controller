#include <string.h>

#ifdef __cplusplus
#include <Arduino.h>
#define DBG_PRINT(x) Serial.print(x)
#define DBG_PRINTLN(x) Serial.println(x)
#define DBG_PRINT_HEX(x) Serial.print((uint32_t)x, HEX)
#define DBG_PRINTLN_HEX(x) Serial.println((uint32_t)x, HEX)
#define DBG_PRINT_INT(x) Serial.print(x)
#define DBG_PRINTLN_INT(x) Serial.println(x)
#else
// For C code, use extern C wrapper functions
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

#include "actions.h"
#include "fonts.h"
#include "images.h"
#include "screens.h"
#include "styles.h"
#include "ui.h"
#include "vars.h"

objects_t objects;
lv_obj_t *tick_value_change_obj;
uint32_t active_theme_index = 0;

void create_screen_main() {
  DBG_PRINTLN("[SCREEN] create_screen_main() called");
  lv_obj_t *obj = lv_obj_create(0);
  DBG_PRINT("[SCREEN] Created screen object: ");
  DBG_PRINTLN_HEX(obj);
  objects.main = obj;
  DBG_PRINT("[SCREEN] objects.main set to: ");
  DBG_PRINTLN_HEX(objects.main);
  lv_obj_set_pos(obj, 0, 0);
  lv_obj_set_size(obj, 800, 480);
  // Remove any theme styles first, then set background color explicitly
  lv_obj_remove_style_all(obj);
  // Set background color explicitly to black
  lv_obj_set_style_bg_color(obj, lv_color_hex(0x000000),
                            LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
  {
    lv_obj_t *parent_obj = obj;
    {
      lv_obj_t *obj = lv_label_create(parent_obj);
      lv_obj_set_pos(obj, 269, 167);
      lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
      lv_label_set_text(obj, "I'm rather fond of team thermocline");
      // Remove theme styles and set text color to white for visibility on black
      // background
      lv_obj_remove_style_all(obj);
      lv_obj_set_style_text_color(obj, lv_color_hex(0xFFFFFF),
                                  LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP,
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    {
      lv_obj_t *obj = lv_img_create(parent_obj);
      lv_obj_set_pos(obj, 355, 197);
      lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
      lv_img_set_src(obj, &img_monk);
    }
  }

  // Invalidate to force redraw
  lv_obj_invalidate(obj);
  DBG_PRINTLN("[SCREEN] Screen invalidated, calling tick_screen_main()");

  tick_screen_main();
  DBG_PRINTLN("[SCREEN] create_screen_main() finished");
}

void tick_screen_main() {}

typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_main,
};
void tick_screen(int screen_index) { tick_screen_funcs[screen_index](); }
void tick_screen_by_id(enum ScreensEnum screenId) {
  tick_screen_funcs[screenId - 1]();
}

void create_screens() {
  DBG_PRINTLN("[SCREEN] create_screens() called");
  lv_disp_t *dispp = lv_disp_get_default();
  DBG_PRINT("[SCREEN] Display pointer: ");
  DBG_PRINTLN_HEX(dispp);
  // Don't set theme - it might override our explicit colors
  // lv_theme_t *theme = lv_theme_default_init(
  //     dispp, lv_palette_main(LV_PALETTE_BLUE),
  //     lv_palette_main(LV_PALETTE_RED), false, LV_FONT_DEFAULT);
  // lv_disp_set_theme(dispp, theme);
  DBG_PRINTLN("[SCREEN] Theme skipped - using explicit colors");

  create_screen_main();
  DBG_PRINTLN("[SCREEN] create_screen_main() completed");
}
