#include "actions.h"

#ifdef __cplusplus
#include <Arduino.h>
#else
// For C code, use extern C wrapper functions
extern void Serial_println(const char *);
#endif

void action_clicked(lv_event_t *e) {
#ifdef __cplusplus
  Serial.println("[ACTION] Button clicked!");
#else
  Serial_println("[ACTION] Button clicked!");
#endif
}
