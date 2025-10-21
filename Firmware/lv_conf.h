/**
 * @file lv_conf.h
 * Configuration file for LVGL v8.3
 */

#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/*====================
   COLOR SETTINGS
 *====================*/

/* Color depth: 1 (1 byte per pixel), 8 (RGB332), 16 (RGB565), 32 (ARGB8888) */
#define LV_COLOR_DEPTH 16

/* Swap the 2 bytes of RGB565 color. Useful if the display has an 8-bit interface (e.g. SPI) */
#define LV_COLOR_16_SWAP 0

/*=========================
   MEMORY SETTINGS
 *=========================*/

/* Size of the memory available for `lv_mem_alloc()` in bytes (>= 2kB) */
#define LV_MEM_CUSTOM 0
#define LV_MEM_SIZE (64U * 1024U)  /* 64KB */

/*====================
   HAL SETTINGS
 *====================*/

/* Default display refresh period. LVG will redraw changed areas with this period time */
#define LV_DISP_DEF_REFR_PERIOD 33  /* 30 FPS */

/* Input device read period in milliseconds */
#define LV_INDEV_DEF_READ_PERIOD 33

/* Use a custom tick source instead of systick or any other timer */
#define LV_TICK_CUSTOM 0

/*================
 * LOG SETTINGS
 *================*/

/* Enable the log module */
#define LV_USE_LOG 1
#if LV_USE_LOG

/* How important log should be added:
 * LV_LOG_LEVEL_TRACE       A lot of logs to give detailed information
 * LV_LOG_LEVEL_INFO        Log important events
 * LV_LOG_LEVEL_WARN        Log if something unwanted happened but didn't cause a problem
 * LV_LOG_LEVEL_ERROR       Only critical issue, when the system may fail
 * LV_LOG_LEVEL_USER        Only logs added by the user
 * LV_LOG_LEVEL_NONE        Do not log anything */
#define LV_LOG_LEVEL LV_LOG_LEVEL_WARN

/* 1: Print the log with 'printf';
 * 0: User need to register a callback with `lv_log_register_print_cb()` */
#define LV_LOG_PRINTF 1

#endif  /*LV_USE_LOG*/

/*=================
 * THEME SETTINGS
 *=================*/

/* A simple, impressive and very complete theme */
#define LV_USE_THEME_DEFAULT 1
#if LV_USE_THEME_DEFAULT

/* 0: Light mode; 1: Dark mode */
#define LV_THEME_DEFAULT_DARK 0

/* 1: Enable grow on press */
#define LV_THEME_DEFAULT_GROW 1

/* Default transition time in [ms] */
#define LV_THEME_DEFAULT_TRANSITION_TIME 80
#endif /*LV_USE_THEME_DEFAULT*/

/*===================
 * FONT USAGE
 *===================*/

/* Montserrat fonts with ASCII range and some symbols */
#define LV_FONT_MONTSERRAT_8  0
#define LV_FONT_MONTSERRAT_10 0
#define LV_FONT_MONTSERRAT_12 0
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_18 0
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_22 0
#define LV_FONT_MONTSERRAT_24 0
#define LV_FONT_MONTSERRAT_26 0
#define LV_FONT_MONTSERRAT_28 0
#define LV_FONT_MONTSERRAT_30 0
#define LV_FONT_MONTSERRAT_32 0
#define LV_FONT_MONTSERRAT_34 0
#define LV_FONT_MONTSERRAT_36 0
#define LV_FONT_MONTSERRAT_38 0
#define LV_FONT_MONTSERRAT_40 0
#define LV_FONT_MONTSERRAT_42 0
#define LV_FONT_MONTSERRAT_44 0
#define LV_FONT_MONTSERRAT_46 0
#define LV_FONT_MONTSERRAT_48 0

/* Default font */
#define LV_FONT_DEFAULT &lv_font_montserrat_14

/*==================
 * LV_USE_FLEX
 *==================*/

/* 1: Enable Flexbox layout */
#define LV_USE_FLEX 1

/*==================
 * LV_USE_GRID
 *==================*/

/* 1: Enable Grid layout */
#define LV_USE_GRID 1

/*==================
 * WIDGETS
 *==================*/

/* The widgets where the objects are built */
#define LV_USE_ARC        1
#define LV_USE_BAR        1
#define LV_USE_BTN        1
#define LV_USE_BTNMATRIX  1
#define LV_USE_CANVAS     1
#define LV_USE_CHECKBOX   1
#define LV_USE_DROPDOWN   1   
#define LV_USE_IMG        1   
#define LV_USE_LABEL      1
#define LV_USE_LINE       1
#define LV_USE_ROLLER     1
#define LV_USE_SLIDER     1
#define LV_USE_SWITCH     1
#define LV_USE_TEXTAREA   1
#define LV_USE_TABLE      1

/*==================
 * EXTRA WIDGETS
 *==================*/

/* Enable extra widgets */
#define LV_USE_ANIMIMG    1
#define LV_USE_CALENDAR   1
#define LV_USE_CHART      1
#define LV_USE_COLORWHEEL 1
#define LV_USE_IMGBTN     1
#define LV_USE_KEYBOARD   1
#define LV_USE_LED        1
#define LV_USE_LIST       1
#define LV_USE_MENU       1
#define LV_USE_METER      1
#define LV_USE_MSGBOX     1
#define LV_USE_SPAN       1
#define LV_USE_SPINBOX    1
#define LV_USE_SPINNER    1
#define LV_USE_TABVIEW    1
#define LV_USE_TILEVIEW   1
#define LV_USE_WIN        1

#endif /*LV_CONF_H*/

