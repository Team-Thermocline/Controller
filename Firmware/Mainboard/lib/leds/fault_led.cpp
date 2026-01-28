#include "fault_led.h"
#include "FreeRTOS.h"
#include "pindefs.h"
#include "task.h"

void faultLedInit() {
  pinMode(PIN_FAULT, OUTPUT);
  digitalWrite(PIN_FAULT, LOW); // Start with LED off
}

void faultLedSet() { digitalWrite(PIN_FAULT, HIGH); }

void faultLedReset() { digitalWrite(PIN_FAULT, LOW); }

void faultLedFlash(uint32_t onTimeMs, uint32_t offTimeMs, uint32_t count) {
  uint32_t cycles = count;
  bool infinite = (count == 0);

  // If count is 0, flash indefinitely (caller should run this in a task)
  // Otherwise, flash the specified number of times
  do {
    digitalWrite(PIN_FAULT, HIGH);
    vTaskDelay(pdMS_TO_TICKS(onTimeMs));
    digitalWrite(PIN_FAULT, LOW);

    if (!infinite) {
      cycles--;
      if (cycles > 0) {
        vTaskDelay(pdMS_TO_TICKS(offTimeMs));
      }
    } else {
      vTaskDelay(pdMS_TO_TICKS(offTimeMs));
    }
  } while (infinite || cycles > 0);
}
