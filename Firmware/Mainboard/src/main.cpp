#include "FreeRTOS.h"
#include "task.h"
#include <Arduino.h>
#include <ArduinoJson.h>

static void blinkTask(void *pvParameters);
static void commTask(void *pvParameters);

static void sendHello();
static void processSerialLine(const String &line, bool &handshakeComplete,
                              bool &schemaError, TickType_t &lastDataTick,
                              uint32_t &sequenceCounter);
static void sendCommandAck(const char *payload);
static void sendData(uint32_t sequenceCounter);

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(50);
  Serial.ignoreFlowControl(true);

  pinMode(LED_BUILTIN, OUTPUT);

  xTaskCreate(blinkTask, "blink", configMINIMAL_STACK_SIZE + 128, nullptr,
              tskIDLE_PRIORITY + 1, nullptr);

  xTaskCreate(commTask, "comm", configMINIMAL_STACK_SIZE + 512, nullptr,
              tskIDLE_PRIORITY + 2, nullptr);
}

void loop() {
  // Loop is running the vTaskScheduler in the background.
  vTaskDelay(pdMS_TO_TICKS(1000));
}

static void blinkTask(void *pvParameters) {
  (void)pvParameters;

  for (;;) {
    digitalWrite(LED_BUILTIN, HIGH);
    vTaskDelay(pdMS_TO_TICKS(250));
    digitalWrite(LED_BUILTIN, LOW);
    vTaskDelay(pdMS_TO_TICKS(250));
  }
}

static void commTask(void *pvParameters) {
  (void)pvParameters;

  TickType_t lastHelloTick = 0;
  TickType_t lastDataTick = 0;
  bool handshakeComplete = false;
  bool schemaError = false;
  uint32_t sequenceCounter = 1;

  for (;;) {
    TickType_t now = xTaskGetTickCount();

    if (!handshakeComplete && !schemaError) {
      if ((now - lastHelloTick) >= pdMS_TO_TICKS(2000)) {
        sendHello();
        lastHelloTick = now;
      }
    }

    while (Serial.available() > 0) {
      String line = Serial.readStringUntil('\n');
      line.trim();
      if (!line.isEmpty()) {
        processSerialLine(line, handshakeComplete, schemaError, lastDataTick,
                          sequenceCounter);
      }
    }

    if (handshakeComplete && !schemaError) {
      if ((now - lastDataTick) >= pdMS_TO_TICKS(2000)) {
        sendData(sequenceCounter++);
        lastDataTick = now;
      }
    }

    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

static void sendHello() {
  StaticJsonDocument<128> doc;
  doc["type"] = "hello";
  doc["fw"] = "1.0.0";
  doc["schema"] = 1;
  serializeJson(doc, Serial);
  Serial.println();
}

static void processSerialLine(const String &line, bool &handshakeComplete,
                              bool &schemaError, TickType_t &lastDataTick,
                              uint32_t &sequenceCounter) {
  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, line);
  if (err) {
    StaticJsonDocument<128> errorDoc;
    errorDoc["type"] = "error";
    errorDoc["message"] = "invalid_json";
    errorDoc["detail"] = err.f_str();
    serializeJson(errorDoc, Serial);
    Serial.println();
    return;
  }

  const char *type = doc["type"] | "";
  if (strcmp(type, "hello_ack") == 0) {
    const char *error = doc["error"] | nullptr;
    if (error && strcmp(error, "unsupported_schema") == 0) {
      schemaError = true;
      handshakeComplete = false;

      StaticJsonDocument<128> resp;
      resp["type"] = "error";
      resp["message"] = "unsupported schema";
      serializeJson(resp, Serial);
      Serial.println();
    } else {
      handshakeComplete = true;
      schemaError = false;
      lastDataTick = xTaskGetTickCount();
      sequenceCounter = 1;
    }
  } else if (strcmp(type, "cmd") == 0) {
    const char *payload = doc["data"] | "";
    sendCommandAck(payload);
  }
}

static void sendCommandAck(const char *payload) {
  StaticJsonDocument<128> doc;
  doc["type"] = "cmd_ack";
  doc["data"] = payload ? payload : "";
  serializeJson(doc, Serial);
  Serial.println();
}

static void sendData(uint32_t sequenceCounter) {
  StaticJsonDocument<128> doc;
  doc["type"] = "data";
  doc["seq"] = sequenceCounter;
  doc["temp"] = 21.4;
  serializeJson(doc, Serial);
  Serial.println();
}

extern "C" {

void vApplicationIdleHook(void) {
  // Idle hook runs when no other tasks are ready.
  __asm volatile("wfi");
}

void vApplicationMallocFailedHook(void) {
  taskDISABLE_INTERRUPTS();
  for (;;) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
  }
}

void vApplicationStackOverflowHook(TaskHandle_t task, char *name) {
  (void)task;
  (void)name;

  taskDISABLE_INTERRUPTS();
  for (;;) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(25);
    digitalWrite(LED_BUILTIN, LOW);
    delay(25);
  }
}

} // extern "C"
