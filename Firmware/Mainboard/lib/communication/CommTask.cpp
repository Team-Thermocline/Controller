#include "CommTask.h"
#include "FreeRTOS.h"
#include "task.h"
#include <ArduinoJson.h>

// Forward declarations for helper functions
static void processSerialLine(HardwareSerial &serial, const String &line,
                              Interface interface, QueueHandle_t commandQueue);
static void sendError(HardwareSerial &serial, const char *message,
                      const char *detail = nullptr);

void commTask(void *pvParameters) {
  // Validate parameters
  CommTaskParams *params = static_cast<CommTaskParams *>(pvParameters);
  if (!params || !params->serialPort || !params->commandQueue) {
    // Invalid parameters, task cannot continue
    for (;;) {
      vTaskDelay(pdMS_TO_TICKS(1000));
      //   TODO: Raise fault
    }
  }

  // Get serial port, interface, and queue
  HardwareSerial &serial = *params->serialPort;
  Interface interface = params->interface;
  QueueHandle_t commandQueue = params->commandQueue;

  // This task's main loop
  for (;;) {
    while (serial.available() > 0) {
      String line = serial.readStringUntil('\n');
      line.trim();
      if (!line.isEmpty()) {
        processSerialLine(serial, line, interface, commandQueue);
      }
    }

    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

static void processSerialLine(HardwareSerial &serial, const String &line,
                              Interface interface, QueueHandle_t commandQueue) {
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, line);
  if (err) {
    // Convert error to string for error message
    String errorStr = String(err.c_str());
    sendError(serial, "invalid_json", errorStr.c_str());
    return;
  }

  // Parse the command type
  const char *type = doc["type"] | "";
  if (strlen(type) == 0) {
    sendError(serial, "missing_type");
    return;
  }

  // Get the data payload (if present)
  JsonObject dataObj = doc["data"];
  String dataStr = "{}";
  if (!dataObj.isNull()) {
    serializeJson(dataObj, dataStr);
  }

  // Create Command object and send to queue
  Command cmd(interface, type, dataStr.c_str());
  if (xQueueSend(commandQueue, &cmd, pdMS_TO_TICKS(100)) != pdTRUE) {
    // Queue is full - send error response
    sendError(serial, "queue_full", "Command queue is full, try again later");
  }
}

static void sendError(HardwareSerial &serial, const char *message,
                      const char *detail) {
  JsonDocument errorDoc;
  errorDoc["type"] = "error";
  errorDoc["message"] = message;
  if (detail) {
    errorDoc["detail"] = detail;
  }
  serializeJson(errorDoc, serial);
  serial.println();
}
