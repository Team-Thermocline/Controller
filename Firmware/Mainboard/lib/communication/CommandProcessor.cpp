#include "CommandProcessor.h"
#include "FreeRTOS.h"
#include "task.h"
#include <ArduinoJson.h>
#include <cstring>
#include <fault_led.h>

// Forward declarations
static HardwareSerial *getSerialForInterface(CommandProcessorParams *params,
                                             Interface interface);
static void sendReply(HardwareSerial *serial, bool success,
                      const JsonObject &data = JsonObject());
static void sendErrorReply(HardwareSerial *serial, const char *message,
                           const char *detail = nullptr);
static void processCommand(CommandProcessorParams *params, const Command &cmd);
static void handleGetStatus(HardwareSerial *serial, const JsonObject &data);

void commandProcessorTask(void *pvParameters) {
  CommandProcessorParams *params =
      static_cast<CommandProcessorParams *>(pvParameters);

  // Validate parameters
  if (!params || !params->commandQueue || !params->usbSerial) {
    // Invalid parameters, task cannot continue, flash fault LED
    faultLedFlash(100, 100, 0); // Flash forever (blocks)
  }

  Command cmd;

  // Main processing loop
  for (;;) {
    // Wait for a command from the queue (blocking)
    if (xQueueReceive(params->commandQueue, &cmd, portMAX_DELAY) == pdTRUE) {
      // Process the command
      processCommand(params, cmd);
      // Command is automatically removed from queue by xQueueReceive
    }
  }
}

static HardwareSerial *getSerialForInterface(CommandProcessorParams *params,
                                             Interface interface) {
  switch (interface) {
  case Interface::USB_SERIAL:
    return params->usbSerial;
  case Interface::HMI_SERIAL:
    return params->hmiSerial;
  default:
    return params->usbSerial; // Default fallback
  }
}

static void sendReply(HardwareSerial *serial, bool success,
                      const JsonObject &data) {
  if (!serial) {
    return;
  }

  JsonDocument replyDoc;
  replyDoc["type"] = "reply";
  JsonObject replyData = replyDoc["data"].to<JsonObject>();
  replyData["success"] = success;

  // Copy any additional data
  if (!data.isNull()) {
    for (JsonPair pair : data) {
      replyData[pair.key()] = pair.value();
    }
  }

  serializeJson(replyDoc, *serial);
  serial->println();
}

static void sendErrorReply(HardwareSerial *serial, const char *message,
                           const char *detail) {
  if (!serial) {
    return;
  }

  JsonDocument errorDoc;
  errorDoc["type"] = "reply";
  JsonObject errorData = errorDoc["data"].to<JsonObject>();
  errorData["success"] = false;
  errorData["error"] = message;
  if (detail) {
    errorData["detail"] = detail;
  }

  serializeJson(errorDoc, *serial);
  serial->println();
}

static void processCommand(CommandProcessorParams *params, const Command &cmd) {
  // Get the serial port for this command's interface
  HardwareSerial *serial = getSerialForInterface(params, cmd.interface);
  if (!serial) {
    // No serial port available for this interface - can't send reply
    return;
  }

  // Parse the command data JSON
  JsonDocument dataDoc;
  DeserializationError err = deserializeJson(dataDoc, cmd.data);
  if (err) {
    sendErrorReply(serial, "invalid_command_data", err.c_str());
    return;
  }

  JsonObject data = dataDoc.as<JsonObject>();

  // Process command based on command name using switch-like pattern
  // Note: C++ doesn't support switch on strings, so we use if/else

  // ************
  // Ping command
  // ************
  if (strcmp(cmd.command, "ping") == 0) {
    // Example: ping command - just respond with success
    JsonDocument replyData;
    replyData["message"] = "pong";
    sendReply(serial, true, replyData.as<JsonObject>());

    // ******************
    // Get status command
    // ******************
  } else if (strcmp(cmd.command, "get_status") == 0) {
    handleGetStatus(serial, data);

    // ***************
    // Unknown command
    // ***************
  } else {
    sendErrorReply(serial, "unknown_command",
                   "Command not recognized or not implemented");
  }
}

static void handleGetStatus(HardwareSerial *serial, const JsonObject &data) {
  (void)data; // Unused for now, but available for future filtering options

  JsonDocument replyData;
  replyData["status"] = "ok";
  replyData["uptime_ms"] = xTaskGetTickCount() * portTICK_PERIOD_MS;
#ifdef REVISION
  replyData["revision"] = REVISION;
#else
  replyData["revision"] = "Unknown";
#endif
#ifdef FWHOST
  replyData["hostname"] = FWHOST;
#else
  replyData["hostname"] = "Unknown";
#endif
#ifdef USERNAME
  replyData["builder"] = USERNAME;
#else
  replyData["builder"] = "Unknown";
#endif
#ifdef BUILD_DATE
  replyData["build_date"] = BUILD_DATE;
#else
  replyData["build_date"] = "Unknown";
#endif
  sendReply(serial, true, replyData.as<JsonObject>());
}
