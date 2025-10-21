#include "ui_simple.h"

// Global UI object definitions
lv_obj_t *ui_Switch2 = NULL;
lv_obj_t *ui_Label3 = NULL;
lv_obj_t *ui_Label5 = NULL;

void ui_init_simple(void) {
    // Create a simple screen
    lv_obj_t *screen = lv_scr_act();
    
    // Create a switch for LED control
    ui_Switch2 = lv_switch_create(screen);
    lv_obj_set_width(ui_Switch2, 50);
    lv_obj_set_height(ui_Switch2, 25);
    lv_obj_set_x(ui_Switch2, 135);
    lv_obj_set_y(ui_Switch2, 50);
    lv_obj_set_align(ui_Switch2, LV_ALIGN_CENTER);
    
    // Create label for temperature
    ui_Label3 = lv_label_create(screen);
    lv_obj_set_width(ui_Label3, 100);
    lv_obj_set_height(ui_Label3, 25);
    lv_obj_set_x(ui_Label3, -50);
    lv_obj_set_y(ui_Label3, 100);
    lv_obj_set_align(ui_Label3, LV_ALIGN_CENTER);
    lv_label_set_text(ui_Label3, "69°C");
    lv_obj_set_style_text_align(ui_Label3, LV_TEXT_ALIGN_CENTER, 0);
    
    // Create label for humidity
    ui_Label5 = lv_label_create(screen);
    lv_obj_set_width(ui_Label5, 100);
    lv_obj_set_height(ui_Label5, 25);
    lv_obj_set_x(ui_Label5, 50);
    lv_obj_set_y(ui_Label5, 100);
    lv_obj_set_align(ui_Label5, LV_ALIGN_CENTER);
    lv_label_set_text(ui_Label5, "20%");
    lv_obj_set_style_text_align(ui_Label5, LV_TEXT_ALIGN_CENTER, 0);
    
    // Add some descriptive labels
    lv_obj_t *temp_label = lv_label_create(screen);
    lv_obj_set_x(temp_label, -50);
    lv_obj_set_y(temp_label, 80);
    lv_obj_set_align(temp_label, LV_ALIGN_CENTER);
    lv_label_set_text(temp_label, "Temp:");
    
    lv_obj_t *hum_label = lv_label_create(screen);
    lv_obj_set_x(hum_label, 50);
    lv_obj_set_y(hum_label, 80);
    lv_obj_set_align(hum_label, LV_ALIGN_CENTER);
    lv_label_set_text(hum_label, "Humidity:");
    
    lv_obj_t *led_label = lv_label_create(screen);
    lv_obj_set_x(led_label, 0);
    lv_obj_set_y(led_label, 20);
    lv_obj_set_align(led_label, LV_ALIGN_CENTER);
    lv_label_set_text(led_label, "LED Control:");
}
