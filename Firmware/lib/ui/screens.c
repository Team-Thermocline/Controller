#include <string.h>

#include "screens.h"
#include "images.h"
#include "fonts.h"
#include "actions.h"
#include "vars.h"
#include "styles.h"
#include "ui.h"

#include <string.h>

objects_t objects;
lv_obj_t *tick_value_change_obj;
uint32_t active_theme_index = 0;

static lv_meter_scale_t * scale0;
static lv_meter_indicator_t * indicator1;

static void event_handler_cb_selector_menu_tempset_spin(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target(e);
        if (tick_value_change_obj != ta) {
            int32_t value = lv_spinbox_get_value(ta);
            set_var_chamber_set(value);
        }
    }
}

void create_screen_main() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.main = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 240);
    {
        lv_obj_t *parent_obj = obj;
        {
            // background
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.background = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_thermocline_logo__mini_);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_FLOATING);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_SCROLLABLE);
        }
        {
            // loading_bar
            lv_obj_t *obj = lv_bar_create(parent_obj);
            objects.loading_bar = obj;
            lv_obj_set_pos(obj, 10, 212);
            lv_obj_set_size(obj, 299, 14);
            lv_bar_set_range(obj, 0, 5);
            lv_bar_set_mode(obj, LV_BAR_MODE_SYMMETRICAL);
            lv_bar_set_value(obj, 1, LV_ANIM_ON);
        }
        {
            lv_obj_t *obj = lv_qrcode_create(parent_obj, 128, lv_color_hex(0xff000000), lv_color_hex(0xffffffff));
            lv_obj_set_pos(obj, 181, 10);
            lv_obj_set_size(obj, 128, 128);
            lv_qrcode_update(obj, "https://team-thermocline.github.io/", 35);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
        }
        {
            lv_obj_t *obj = lv_textarea_create(parent_obj);
            lv_obj_set_pos(obj, 10, 10);
            lv_obj_set_size(obj, 150, 191);
            lv_textarea_set_max_length(obj, 128);
            lv_textarea_set_text(obj, "Booting...\nCalibrating ext...\nCalibrating int...\nDone!");
            lv_textarea_set_one_line(obj, false);
            lv_textarea_set_password_mode(obj, false);
        }
    }
    
    tick_screen_main();
}

void tick_screen_main() {
}

void create_screen_selector_menu() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.selector_menu = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 240);
    {
        lv_obj_t *parent_obj = obj;
        {
            // background_1
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.background_1 = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_thermocline_logo__mini_);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_FLOATING);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_SCROLLABLE);
        }
        {
            // footer
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.footer = obj;
            lv_obj_set_pos(obj, 0, 202);
            lv_obj_set_size(obj, 320, 38);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // footerchart
                    lv_obj_t *obj = lv_chart_create(parent_obj);
                    objects.footerchart = obj;
                    lv_obj_set_pos(obj, -15, -14);
                    lv_obj_set_size(obj, 120, 36);
                }
                {
                    lv_obj_t *obj = lv_img_create(parent_obj);
                    lv_obj_set_pos(obj, 105, -13);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_img_set_src(obj, &img_thermocline_logo__large_);
                    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
                }
            }
        }
        {
            // tabview_holder
            lv_obj_t *obj = lv_tabview_create(parent_obj, LV_DIR_TOP, 32);
            objects.tabview_holder = obj;
            lv_obj_set_pos(obj, 5, 5);
            lv_obj_set_size(obj, 310, 190);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // set_mode_tab
                    lv_obj_t *obj = lv_tabview_add_tab(parent_obj, "Setpoint");
                    objects.set_mode_tab = obj;
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            // start_btn
                            lv_obj_t *obj = lv_btn_create(parent_obj);
                            objects.start_btn = obj;
                            lv_obj_set_pos(obj, 166, 66);
                            lv_obj_set_size(obj, 100, 50);
                            lv_obj_set_style_text_font(obj, &lv_font_montserrat_26, LV_PART_MAIN | LV_STATE_DEFAULT);
                            {
                                lv_obj_t *parent_obj = obj;
                                {
                                    lv_obj_t *obj = lv_label_create(parent_obj);
                                    objects.obj0 = obj;
                                    lv_obj_set_pos(obj, 0, 0);
                                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                                    lv_label_set_long_mode(obj, LV_LABEL_LONG_CLIP);
                                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_label_set_text(obj, "");
                                }
                            }
                        }
                        {
                            lv_obj_t *obj = lv_meter_create(parent_obj);
                            objects.obj1 = obj;
                            lv_obj_set_pos(obj, -4, -6);
                            lv_obj_set_size(obj, 146, 144);
                            {
                                lv_meter_scale_t *scale = lv_meter_add_scale(obj);
                                scale0 = scale;
                                lv_meter_set_scale_ticks(obj, scale, 49, 1, 5, lv_color_hex(0xffa0a0a0));
                                lv_meter_set_scale_major_ticks(obj, scale, 8, 3, 10, lv_color_hex(0xff000000), 10);
                                lv_meter_set_scale_range(obj, scale, -40, 80, 270, 120);
                                {
                                    lv_meter_indicator_t *indicator = lv_meter_add_needle_line(obj, scale, 3, lv_color_hex(0xff0000ff), -5);
                                    indicator1 = indicator;
                                }
                            }
                            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE);
                        }
                        {
                            // tempset_spin
                            lv_obj_t *obj = lv_spinbox_create(parent_obj);
                            objects.tempset_spin = obj;
                            lv_obj_set_pos(obj, 151, -6);
                            lv_obj_set_size(obj, 130, 44);
                            lv_spinbox_set_digit_format(obj, 4, 2);
                            lv_spinbox_set_range(obj, -40, 70);
                            lv_spinbox_set_rollover(obj, false);
                            lv_spinbox_set_step(obj, 1);
                            lv_obj_add_event_cb(obj, event_handler_cb_selector_menu_tempset_spin, LV_EVENT_ALL, 0);
                            lv_obj_set_style_text_font(obj, &ui_font_archivo, LV_PART_MAIN | LV_STATE_DEFAULT);
                        }
                    }
                }
                {
                    // set_curve_tab
                    lv_obj_t *obj = lv_tabview_add_tab(parent_obj, "Curve");
                    objects.set_curve_tab = obj;
                }
                {
                    // settings_tab
                    lv_obj_t *obj = lv_tabview_add_tab(parent_obj, "Settings");
                    objects.settings_tab = obj;
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_img_create(parent_obj);
                            lv_obj_set_pos(obj, 190, 206);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_img_set_src(obj, &img_monk);
                        }
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.obj2 = obj;
                            lv_obj_set_pos(obj, 64, -6);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_label_set_text(obj, "");
                        }
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, -2, -6);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_label_set_text(obj, "Version");
                        }
                    }
                }
            }
        }
    }
    
    tick_screen_selector_menu();
}

void tick_screen_selector_menu() {
    {
        const char *new_val = get_var_setpoint_ready();
        const char *cur_val = lv_label_get_text(objects.obj0);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.obj0;
            lv_label_set_text(objects.obj0, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        lv_meter_indicator_t *indicator;
        
        lv_ll_t *indicators = &((lv_meter_t *)objects.obj1)->indicator_ll;
        int index = 0;
        for (indicator = _lv_ll_get_tail(indicators); index > 0 && indicator != NULL; indicator = _lv_ll_get_prev(indicators, indicator), index--);
        
        if (indicator) {
            int32_t new_val = get_var_chamber_current();
            int32_t cur_val = indicator->start_value;
            if (new_val != cur_val) {
                tick_value_change_obj = objects.obj1;
                lv_meter_set_indicator_value(objects.obj1, indicator, new_val);
                tick_value_change_obj = NULL;
            }
        }
    }
    {
        int32_t new_val = get_var_chamber_set();
        int32_t cur_val = lv_spinbox_get_value(objects.tempset_spin);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.tempset_spin;
            lv_spinbox_set_value(objects.tempset_spin, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_version();
        const char *cur_val = lv_label_get_text(objects.obj2);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.obj2;
            lv_label_set_text(objects.obj2, new_val);
            tick_value_change_obj = NULL;
        }
    }
}



typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_main,
    tick_screen_selector_menu,
};
void tick_screen(int screen_index) {
    tick_screen_funcs[screen_index]();
}
void tick_screen_by_id(enum ScreensEnum screenId) {
    tick_screen_funcs[screenId - 1]();
}

void create_screens() {
    lv_disp_t *dispp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), true, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    
    create_screen_main();
    create_screen_selector_menu();
}
