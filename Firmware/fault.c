#include "fault.h"

#include "FreeRTOS.h"
#include "globals.h"
#include "queue.h"

#define FAULT_QUEUE_LEN 4

static QueueHandle_t s_fault_queue;
static void (*s_on_fault)(fault_code_t) = NULL;

void fault_init(void) {
  s_fault_queue = xQueueCreate(FAULT_QUEUE_LEN, sizeof(fault_code_t));
}

void fault_raise(fault_code_t code) {
  if (s_fault_queue != NULL) {
    (void)xQueueSend(s_fault_queue, &code, 0);
  }
}

void fault_register_on_fault(void (*handler)(fault_code_t)) {
  s_on_fault = handler;
}

void fault_process(void) {
  if (s_fault_queue == NULL) {
    return;
  }
  fault_code_t code;
  if (xQueueReceive(s_fault_queue, &code, 0) == pdTRUE) {
    FAULT = code;
    if (s_on_fault != NULL) {
      s_on_fault(code);
    }
  }
}
