#ifndef UI_SIMPLE_H
#define UI_SIMPLE_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Global UI object declarations
extern lv_obj_t *ui_Switch2;
extern lv_obj_t *ui_Label3;
extern lv_obj_t *ui_Label5;

// Function declarations
void ui_init_simple(void);

#ifdef __cplusplus
}
#endif

#endif // UI_SIMPLE_H
