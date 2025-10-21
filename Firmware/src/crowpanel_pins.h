/*
  CrowPanel Pico HMI 4.3-inch Pin Definitions
  
  Based on Elecrow specifications:
  https://www.elecrow.com/wiki/CrowPanel_Pico_HMI_Display-4.3.html
*/

#ifndef CROWPANEL_PINS_H
#define CROWPANEL_PINS_H

// Display SPI Pins (from schematic)
#define DISPLAY_MOSI_PIN    19
#define DISPLAY_MISO_PIN    16
#define DISPLAY_SCK_PIN     18
#define DISPLAY_CS_PIN      17
#define DISPLAY_DC_PIN      20
#define DISPLAY_RST_PIN     21
#define DISPLAY_BACKLIGHT   22

// Touch Screen Pins (Capacitive - I2C interface)
#define TOUCH_SDA_PIN       2
#define TOUCH_SCL_PIN       3
#define TOUCH_INT_PIN       4

// Communication Pins
#define UART0_RX_PIN        1
#define UART0_TX_PIN        0
#define UART1_RX_PIN        5
#define UART1_TX_PIN        4

// Available GPIO Pins
#define GPIO_6_PIN          6
#define GPIO_7_PIN          7
#define GPIO_16_PIN         16
#define GPIO_17_PIN         17
#define GPIO_22_PIN         22
#define GPIO_26_PIN         26
#define GPIO_27_PIN         27
#define GPIO_28_PIN         28

// ADC Pins (GP26, GP27 can also be used as ADC)
#define ADC_26_PIN          26
#define ADC_27_PIN          27

// Built-in LED (if available)
#define LED_PIN             25

// TF Card Pins (if needed)
#define TF_CS_PIN           13
#define TF_MOSI_PIN         11
#define TF_MISO_PIN         12
#define TF_SCK_PIN          10

// Display Configuration
#define DISPLAY_WIDTH       320
#define DISPLAY_HEIGHT      240
#define DISPLAY_ROTATION    0

// Touch Configuration
#define TOUCH_I2C_ADDRESS   0x38  // Typical capacitive touch controller address

// I2C Configuration
#define I2C_SDA_PIN         TOUCH_SDA_PIN
#define I2C_SCL_PIN         TOUCH_SCL_PIN

#endif // CROWPANEL_PINS_H
