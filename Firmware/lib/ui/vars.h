#ifndef EEZ_LVGL_UI_VARS_H
#define EEZ_LVGL_UI_VARS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// enum declarations



// Flow global variables

enum FlowGlobalVariables {
    FLOW_GLOBAL_VARIABLE_VERSION = 0,
    FLOW_GLOBAL_VARIABLE_CHAMBER_CURRENT = 1,
    FLOW_GLOBAL_VARIABLE_CHAMBER_SET = 2,
    FLOW_GLOBAL_VARIABLE_SETPOINT_READY = 3
};

// Native global variables

extern const char *get_var_version();
extern void set_var_version(const char *value);
extern double get_var_chamber_current();
extern void set_var_chamber_current(double value);
extern double get_var_chamber_set();
extern void set_var_chamber_set(double value);
extern const char *get_var_setpoint_ready();
extern void set_var_setpoint_ready(const char *value);


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_VARS_H*/