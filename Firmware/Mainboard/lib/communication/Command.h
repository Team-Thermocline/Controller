#ifndef COMMAND_H
#define COMMAND_H

#include <cstdint>
#include <cstring>

/**
 * @brief Communication Interfaces enum
 */
enum class Interface : uint8_t {
  USB_SERIAL = 0, // The virtual serial port we host via the 2040
  HMI_SERIAL = 1, // The internal serial port we use for the HMI
  // Could add more.
  UNKNOWN = 255
};

/**
 * @brief Queueable command structure
 *
 * You can use almost anything in a FreeRTOS queue,
 * but this structure pretty much has everything we want.
 * Feel free to edit.
 * @author Joe
 */
struct Command {
  Interface interface;
  char command[64]; // Command name/type
  char data[512];   // JSON data/arguments as string

  /**
   * @brief Default constructor - initializes with empty values
   */
  Command() : interface(Interface::UNKNOWN) {
    command[0] = '\0';
    data[0] = '\0';
  }

  /**
   * @brief Constructor with all parameters
   * @param iface The interface this command was received on
   * @param cmd The command name/type (will be truncated if > 63 chars)
   * @param jsonData The JSON data/arguments as a string (will be truncated if >
   * 511 chars)
   */
  Command(Interface iface, const char *cmd, const char *jsonData)
      : interface(iface) {
    // Copy command name (ensure null termination)
    strncpy(command, cmd ? cmd : "", sizeof(command) - 1);
    command[sizeof(command) - 1] = '\0';

    // Copy JSON data (ensure null termination)
    strncpy(data, jsonData ? jsonData : "", sizeof(data) - 1);
    data[sizeof(data) - 1] = '\0';
  }

  /**
   * @brief Copy constructor
   */
  Command(const Command &other) : interface(other.interface) {
    strncpy(command, other.command, sizeof(command));
    strncpy(data, other.data, sizeof(data));
  }

  /**
   * @brief Assignment operator
   */
  Command &operator=(const Command &other) {
    if (this != &other) {
      interface = other.interface;
      strncpy(command, other.command, sizeof(command));
      strncpy(data, other.data, sizeof(data));
    }
    return *this;
  }

  /**
   * @brief Check if command is valid (has a command name)
   */
  bool isValid() const { return command[0] != '\0'; }
};

// Static assertion to ensure Command is suitable for FreeRTOS queue
static_assert(sizeof(Command) <= 1024,
              "Command structure is too large for efficient queue usage");

#endif // COMMAND_H
