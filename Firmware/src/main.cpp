#include <PicoDVI.h>                  // Core display & graphics library
#include <lvgl.h>
#include "ui_simple.h"
#include "touch.h"
#include <Arduino.h>
#include <Wire.h>
// #include <Crowbits_DHT20.h>


int led_flag=0;
//Initializes the display class
DVIGFX16 display(  DVI_RES_320x240p60, picodvi_dvi_cfg);

// Initializes the DHT20 sensor class
// Crowbits_DHT20 dht20;

//Set the LVGL's 4.3inch Pico screen parameters

static const uint16_t screenWidth = 320;
static const uint16_t screenHeight = 240;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[ screenWidth * screenHeight / 10];


//Define the temperature variable temp and the humidity variable hum for the temperature and humidity sensor
int  temp=69;
int  hum=20;
/*Defines a time interval variable for a non-blocking interval operation
   previousMillis：the sensor data was last updated
   interval：Everytime the sensor data is updated
*/
unsigned long previousMillis = 0;   // Stores last time temperature was published
const long interval = 1000;        // Interval at which to publish sensor readings

/*Define the variables that store the converted temperature and humidity data
   buffer_t[10]：A variable that stores the converted temperature data
   buffer_h[10]：A variable that stores the converted humidity data
*/
char buffer_t[10];
char buffer_h[10];

/*Define pins for touch sensors and LED lights
   ledPin：LED pin
*/
const int ledPin = 28;

/*LVGL displays the callback function
   Parameter immutable
*/
/* Display flushing */
void my_disp_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p )
{
  uint32_t w = ( area->x2 - area->x1 + 1 );
  uint32_t h = ( area->y2 - area->y1 + 1 );
  display.drawRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
  lv_disp_flush_ready( disp );
}
/*LVGL touches the callback function
   Parameter immutable
*/
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
  if (touch_has_signal()) {
    if (touch_touched()) {
      data->state = LV_INDEV_STATE_PRESSED;

      /*Set the coordinates*/
      data->point.x = touch_last_x;
      data->point.y = touch_last_y;
      // Serial2.print( "Data x " );
      // Serial2.println( data->point.x );
      // Serial2.print( "Data y " );
      // Serial2.println( data->point.y );
    } else if (touch_released()) {
      data->state = LV_INDEV_STATE_RELEASED;
    }
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}
void setup()
{
      /*Sets the output mode of the TouchPin and ledPin 
     TouchPin：INPUT 
     ledPin：OUTPUT 
  */
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW); 

  //Initialize the baud rate of the USB virtual serial port to 115200
  Serial.begin(115200); /*serial init */
      /*Due to the different pins of different boards, the SDA and SCL pins of the IIC need to be reset
     Wire.setSDA(2)：Set SDA pin to 2
     Wire.setSCL(3)：Set SCL pin to 3
  */
  Wire.setSDA(2);
  Wire.setSCL(3);
  //Initialize IIC
  Wire.begin();

  //Turn on the screen backlight
  pinMode(24, OUTPUT);
  digitalWrite(24, LOW); // Enable backlight
  //Initialize screen
  if (!display.begin()) {

  }
  //Set the rotation direction to 0
  display.setRotation(0);  // Takes effect on next drawing command
  //Fill the screen in black
  display.fillScreen(0x0000);

  delay(1000);

  //Initialize touch screen
  touch_init(screenWidth, screenHeight);
  //Initialize lvgl
  lv_init();
    /*Initializes the LCGL buffer
     draw_buf_t *draw_buf: A pointer to the initialized display buffer
     uint8_t *buf1: The address of the memory buffer
     lv_color_t *NULL1: The address of the optional second buffer
     screenWidth * screenHeight / 10: The size of the buffer, usually in bytes
  */
  lv_disp_draw_buf_init( &draw_buf, buf1, NULL, screenWidth * screenHeight / 10 );
  //Register LVGL to display callback functions
  /*Initialize the display*/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init( &disp_drv );
  /*Change the following line to your display resolution*/
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  // disp_drv.full_refresh = 1;
  lv_disp_drv_register( &disp_drv );
  //Register the LVGL touch callback function
  /*Initialize the (dummy) input device driver*/
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init( &indev_drv );
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register( &indev_drv );

  //Initialize the UI interface
  ui_init_simple();        //LVGL UI init
  Serial.println( "Setup done" );
  delay(200);


}



void loop()
{

  //According to the button status to determine whether to light
    if (led_flag == 1)
    {
      digitalWrite(ledPin, HIGH);
      lv_obj_add_state(ui_Switch2, LV_STATE_CHECKED);

    }
    else
    {
      digitalWrite(ledPin, LOW);      
      lv_obj_clear_state(ui_Switch2, LV_STATE_CHECKED);
    }
//The LVGL timer task function is called to update the status of the UI
  lv_timer_handler();
  delay(10);
      //Get touch sensor data without blocking, if the interval is too short it will cause the program to stall

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval)
    {
      previousMillis = currentMillis;
      // Reading humidity data
      // hum = (int)dht20.getHumidity();
      // Read temperature data
      // temp = (int)dht20.getTemperature();
      Serial.print("hum:");
      Serial.println( hum);
      Serial.print("temp:");
      Serial.println( temp);
      /*Convert temperature and humidity values to update UI. itoa function parameters are described as follows:
       hum，temp：The value to be converted
       buffer_h，buffer_h：Stores the converted variable
       10：Stores the size of the converted variable
    */
      itoa(hum, buffer_h, 10);
      itoa(temp, buffer_t, 10);
      //Update the UI data
      lv_label_set_text(ui_Label3, buffer_t);
      lv_label_set_text(ui_Label5, buffer_h);
      //Check whether the temperature and humidity data is successfully read
      if (isnan(temp) || isnan(hum))
      {
        Serial.println(F("Failed to read from DHT20 sensor!"));
        return;
      }



    }
}