#include <PicoDVI.h>                  // Core display & graphics library
#include <lvgl.h>
#include "ui.h"
#include "touch.h"
#include <Arduino.h>
#include <Wire.h>
#include <Crowbits_DHT20.h>

int led_flag=0;
//初始化显示类
DVIGFX16 display(  DVI_RES_320x240p60, picodvi_dvi_cfg);

// 初始化DHT20传感器类
Crowbits_DHT20 dht20;

//设置LVGL的4.3Pico屏幕参数

static const uint16_t screenWidth = 320;
static const uint16_t screenHeight = 240;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[ screenWidth * screenHeight / 10];


//定义温湿度传感器的温度变量temp，和湿度变量hum
int  temp;
int  hum;
/*定义无阻塞间隔操作的时间间隔变量，
   previousMillis：最后一次更新传感器数据的时间
   interval：每次更新传感器数据的时间
*/
unsigned long previousMillis = 0;   // Stores last time temperature was published
const long interval = 1000;        // Interval at which to publish sensor readings

/*定义储存温湿度数据转换后的数据的变量
   buffer_t[10]：储存转换后温度的数据的变量
   buffer_h[10]：储存转换后湿度的数据的变量
*/
char buffer_t[10];
char buffer_h[10];

/*定义触摸传感器和LED灯的引脚
   ledPin：LED灯的引脚
*/
const int ledPin = 28;

/*LVGL显示回调函数
   参数不可改
*/
/* Display flushing */
void my_disp_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p )
{
  uint32_t w = ( area->x2 - area->x1 + 1 );
  uint32_t h = ( area->y2 - area->y1 + 1 );
  display.drawRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
  lv_disp_flush_ready( disp );
}
/*LVGL触摸回调函数
   参数不可改
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
      /*设置TouchPin和ledPin引脚的输出模式
     TouchPin：INPUT 输入模式
     ledPin：OUTPUT 输出模式
  */
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW); 

  //初始化USB虚拟串口波特率115200
  Serial.begin(115200); /*serial init */
      /*由于不同板子引脚不同，需要重新设置IIC的SDA和SCL脚
     Wire.setSDA(2)：设置SDA脚为2
     Wire.setSCL(3)：设置SCL脚为3
  */
  Wire.setSDA(2);
  Wire.setSCL(3);
  //初始化IIC
  Wire.begin();

  //打开屏幕背光
  pinMode(24, OUTPUT);
  digitalWrite(24, LOW); // Enable backlight
  //初始化屏幕
  if (!display.begin()) {

  }
  //设置旋转方向为 0
  display.setRotation(0);  // Takes effect on next drawing command
  //填充屏幕为黑色
  display.fillScreen(0x0000);

  delay(1000);

  //触摸屏初始化
  touch_init(screenWidth, screenHeight);
  //lvgl 初始化
  lv_init();
    /*初始化LCGL缓冲区
     draw_buf_t *draw_buf: 指向已初始化的显示缓冲区的指针。
     uint8_t *buf1: 内存缓冲区的地址。
     lv_color_t *NULL1: 可选的第二个缓冲区的地址。
     screenWidth * screenHeight / 10: 缓冲区的大小，通常以字节为单位。
  */
  lv_disp_draw_buf_init( &draw_buf, buf1, NULL, screenWidth * screenHeight / 10 );
  //注册LVGL显示回调函数
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
  //注册LVGL触摸回调函数
  /*Initialize the (dummy) input device driver*/
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init( &indev_drv );
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register( &indev_drv );

  //初始化UI界面
  ui_init();        //LVGL UI init
  Serial.println( "Setup done" );
  delay(200);


}



void loop()
{

  //根据按钮状态来决定是否亮灯
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
//LVGL定时任务函数，调用该函数来更新UI的状态
  lv_timer_handler();
  delay(10);
      //无阻塞获取触摸传感器数据，如果间隔太短会导致程序卡死

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval)
    {
      previousMillis = currentMillis;
      // 读取湿度数据
      hum = (int)dht20.getHumidity();
      // 读取温度数据
      temp = (int)dht20.getTemperature();
      Serial.print("hum:");
      Serial.println( hum);
      Serial.print("temp:");
      Serial.println( temp);
      /*转换温湿度的数值，用于更新UI，itoa函数参数介绍：
       hum，temp：要转化的数值
       buffer_h，buffer_h：储存转化后的变量
       10：储存转化后变量的大小
    */
      itoa(hum, buffer_h, 10);
      itoa(temp, buffer_t, 10);
      //更新UI的数据
      lv_label_set_text(ui_Label3, buffer_t);
      lv_label_set_text(ui_Label5, buffer_h);
      //判断是否读取温湿度数据成功
      if (isnan(temp) || isnan(hum))
      {
        Serial.println(F("Failed to read from DHT20 sensor!"));
        return;
      }
  

  
    }
}
