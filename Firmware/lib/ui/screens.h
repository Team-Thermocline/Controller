#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *selector_menu;
    lv_obj_t *background;
    lv_obj_t *loading_bar;
    lv_obj_t *background_1;
    lv_obj_t *footer;
    lv_obj_t *footerchart;
    lv_obj_t *tabview_holder;
    lv_obj_t *set_mode_tab;
    lv_obj_t *start_btn;
    lv_obj_t *tempset_spin;
    lv_obj_t *set_curve_tab;
    lv_obj_t *settings_tab;
    lv_obj_t *obj0;
    lv_obj_t *obj1;
    lv_obj_t *obj2;
} objects_t;

extern objects_t objects;

enum ScreensEnum {
    SCREEN_ID_MAIN = 1,
    SCREEN_ID_SELECTOR_MENU = 2,
};

void create_screen_main();
void tick_screen_main();

void create_screen_selector_menu();
void tick_screen_selector_menu();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/