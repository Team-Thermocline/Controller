/*******************************************************************************
 * Touch libraries:
 * FT6X36: https://github.com/strange-v/FT6X36.git
 * GT911: https://github.com/TAMCTec/gt911-arduino.git
 * XPT2046: https://github.com/PaulStoffregen/XPT2046_Touchscreen.git
 ******************************************************************************/

/* uncomment for FT6X36 */
// #define TOUCH_FT6X36
// #define TOUCH_FT6X36_SCL 19
// #define TOUCH_FT6X36_SDA 18
// #define TOUCH_FT6X36_INT 39
// #define TOUCH_SWAP_XY
// #define TOUCH_MAP_X1 480
// #define TOUCH_MAP_X2 0
// #define TOUCH_MAP_Y1 0
// #define TOUCH_MAP_Y2 320

/* uncomment for GT911 */
 #define TOUCH_GT911
 #define TOUCH_GT911_SCL 20//20
 #define TOUCH_GT911_SDA 19//19
 #define TOUCH_GT911_INT -1//-1
 #define TOUCH_GT911_RST -1//38
 #define TOUCH_GT911_ROTATION ROTATION_NORMAL
 #define TOUCH_MAP_X1 800//480
 #define TOUCH_MAP_X2 0
 #define TOUCH_MAP_Y1 480//272
 #define TOUCH_MAP_Y2 0

/* uncomment for XPT2046 */
// #define TOUCH_XPT2046
// #define TOUCH_XPT2046_SCK 12
// #define TOUCH_XPT2046_MISO 13
// #define TOUCH_XPT2046_MOSI 11
// #define TOUCH_XPT2046_CS 38
// #define TOUCH_XPT2046_INT 18
// #define TOUCH_XPT2046_ROTATION 0
// #define TOUCH_MAP_X1 4000//4000
// #define TOUCH_MAP_X2 100 //100
// #define TOUCH_MAP_Y1 100//100
// #define TOUCH_MAP_Y2 4000//4000

int touch_last_x = 0, touch_last_y = 0;

#if defined(TOUCH_FT6X36)
#include <Wire.h>
#include <FT6X36.h>
FT6X36 ts(&Wire, TOUCH_FT6X36_INT);
bool touch_touched_flag = true, touch_released_flag = true;

#elif defined(TOUCH_GT911)
#include <Wire.h>
#include <TAMC_GT911.h>
TAMC_GT911 ts = TAMC_GT911(TOUCH_GT911_SDA, TOUCH_GT911_SCL, TOUCH_GT911_INT, TOUCH_GT911_RST, max(TOUCH_MAP_X1, TOUCH_MAP_X2), max(TOUCH_MAP_Y1, TOUCH_MAP_Y2));

#elif defined(TOUCH_XPT2046)
#include <XPT2046_Touchscreen.h>
#include <SPI.h>
XPT2046_Touchscreen ts(TOUCH_XPT2046_CS, TOUCH_XPT2046_INT);
//T2046_Touchscreen ts(TOUCH_XPT2046_CS);  // Param 2 - NULL - No interrupts 



#endif

#if defined(TOUCH_FT6X36)
void touch(TPoint p, TEvent e)
{
  if (e != TEvent::Tap && e != TEvent::DragStart && e != TEvent::DragMove && e != TEvent::DragEnd)
  {
    return;
  }
  // translation logic depends on screen rotation
#if defined(TOUCH_SWAP_XY)
  touch_last_x = map(p.y, TOUCH_MAP_X1, TOUCH_MAP_X2, 0, lcd.width());
  touch_last_y = map(p.x, TOUCH_MAP_Y1, TOUCH_MAP_Y2, 0, lcd.height());
#else
  touch_last_x = map(p.x, TOUCH_MAP_X1, TOUCH_MAP_X2, 0, lcd.width());
  touch_last_y = map(p.y, TOUCH_MAP_Y1, TOUCH_MAP_Y2, 0, lcd.height());
#endif
  switch (e)
  {
  case TEvent::Tap:
    Serial.println("Tap");
    touch_touched_flag = true;
    touch_released_flag = true;
    break;
  case TEvent::DragStart:
    Serial.println("DragStart");
    touch_touched_flag = true;
    break;
  case TEvent::DragMove:
    Serial.println("DragMove");
    touch_touched_flag = true;
    break;
  case TEvent::DragEnd:
    Serial.println("DragEnd");
    touch_released_flag = true;
    break;
  default:
    Serial.println("UNKNOWN");
    break;
  }
}
#endif

void touch_init()
{
#if defined(TOUCH_FT6X36)
  Wire.begin(TOUCH_FT6X36_SDA, TOUCH_FT6X36_SCL);
  ts.begin();
  ts.registerTouchHandler(touch);

#elif defined(TOUCH_GT911)
  Serial.println("[TOUCH] Starting GT911 init...");
  Serial.flush();
  delay(100);
  
  Wire.begin(TOUCH_GT911_SDA, TOUCH_GT911_SCL);
  Serial.print("[TOUCH] Initializing GT911 on SDA=");
  Serial.print(TOUCH_GT911_SDA);
  Serial.print(" SCL=");
  Serial.println(TOUCH_GT911_SCL);
  Serial.flush();
  delay(100);
  
  // Check I2C scanner to see if GT911 is present
  // GT911 typically uses address 0x14 or 0x5D
  Serial.println("[TOUCH] Scanning I2C bus for GT911...");
  Serial.flush();
  delay(200);
  
  byte error;
  int nDevices = 0;
  
  // Check common GT911 addresses first
  uint8_t gt911_addresses[] = {0x14, 0x5D, 0xBA, 0x28};
  Serial.println("[TOUCH] Checking known GT911 addresses:");
  Serial.flush();
  
  for(int i = 0; i < 4; i++) {
    uint8_t addr = gt911_addresses[i];
    Wire.beginTransmission(addr);
    error = Wire.endTransmission();
    Serial.print("[TOUCH] Address 0x");
    if (addr < 16) Serial.print("0");
    Serial.print(addr, HEX);
    Serial.print(": ");
    if (error == 0) {
      Serial.println("FOUND!");
      nDevices++;
    } else {
      Serial.print("Not found (error ");
      Serial.print(error);
      Serial.println(")");
    }
    Serial.flush();
    delay(50);
  }
  
  // Full scan
  Serial.println("[TOUCH] Full I2C scan (1-126)...");
  Serial.flush();
  delay(100);
  for(uint8_t address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("[TOUCH] I2C device at 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.println(" !");
      Serial.flush();
      nDevices++;
      delay(20);
    }
  }
  
  Serial.print("[TOUCH] I2C scan complete. Found ");
  Serial.print(nDevices);
  Serial.println(" device(s) total");
  Serial.flush();
  if (nDevices == 0) {
    Serial.println("[TOUCH] ERROR: No I2C devices found! Check SDA/SCL pins!");
    Serial.flush();
  }
  delay(300);
  
  Serial.println("[TOUCH] Calling ts.begin()...");
  Serial.flush();
  ts.begin();
  Serial.println("[TOUCH] ts.begin() completed");
  Serial.flush();
  delay(100);
  
  Serial.println("[TOUCH] Setting rotation...");
  Serial.flush();
  ts.setRotation(TOUCH_GT911_ROTATION);
  Serial.println("[TOUCH] GT911 initialized and rotation set");
  Serial.flush();

#elif defined(TOUCH_XPT2046)
  SPI.begin(TOUCH_XPT2046_SCK, TOUCH_XPT2046_MISO, TOUCH_XPT2046_MOSI, TOUCH_XPT2046_CS);
  ts.begin();
  ts.setRotation(TOUCH_XPT2046_ROTATION);

#endif
}

bool touch_has_signal()
{
#if defined(TOUCH_FT6X36)
  ts.loop();
  return touch_touched_flag || touch_released_flag;

#elif defined(TOUCH_GT911)
  return true;

#elif defined(TOUCH_XPT2046)
  return ts.tirqTouched();

#else
  return false;
#endif
}

bool touch_touched()
{
#if defined(TOUCH_FT6X36)
  if (touch_touched_flag)
  {
    touch_touched_flag = false;
    return true;
  }
  else
  {
    return false;
  }

#elif defined(TOUCH_GT911)
  static int read_count = 0;
  static unsigned long last_read_debug = 0;
  static bool last_touched_state = false;
  read_count++;
  
  // Debug read status every 2 seconds
  if (millis() - last_read_debug > 2000) {
    Serial.print("[TOUCH] GT911 read() called ");
    Serial.print(read_count);
    Serial.print(" times, isTouched=");
    Serial.println(ts.isTouched ? "TRUE" : "FALSE");
    Serial.flush();
    last_read_debug = millis();
    read_count = 0;
  }
  
  ts.read();
  
  // Always print the isTouched status when it changes
  if (ts.isTouched != last_touched_state) {
    Serial.print("[TOUCH] GT911 isTouched changed: ");
    Serial.print(last_touched_state ? "TRUE" : "FALSE");
    Serial.print(" -> ");
    Serial.println(ts.isTouched ? "TRUE" : "FALSE");
    Serial.flush();
    last_touched_state = ts.isTouched;
  }
  
  if (ts.isTouched)
  {
#if defined(TOUCH_SWAP_XY)
    touch_last_x = map(ts.points[0].y, TOUCH_MAP_X1, TOUCH_MAP_X2, 0, lcd.width() - 1);
    touch_last_y = map(ts.points[0].x, TOUCH_MAP_Y1, TOUCH_MAP_Y2, 0, lcd.height() - 1);
#else
    touch_last_x = map(ts.points[0].x, TOUCH_MAP_X1, TOUCH_MAP_X2, 0, lcd.width() - 1);
    touch_last_y = map(ts.points[0].y, TOUCH_MAP_Y1, TOUCH_MAP_Y2, 0, lcd.height() - 1);
#endif
    Serial.print("[TOUCH] GT911 TOUCHED! Raw X=");
    Serial.print(ts.points[0].x);
    Serial.print(" Raw Y=");
    Serial.print(ts.points[0].y);
    Serial.print(" Mapped X=");
    Serial.print(touch_last_x);
    Serial.print(" Mapped Y=");
    Serial.println(touch_last_y);
    return true;
  }
  else
  {
    return false;
  }

#elif defined(TOUCH_XPT2046)
  if (ts.touched())
  {
    TS_Point p = ts.getPoint();
#if defined(TOUCH_SWAP_XY)
    touch_last_x = map(p.y, TOUCH_MAP_X1, TOUCH_MAP_X2, 0, lcd.width() - 1);
    touch_last_y = map(p.x, TOUCH_MAP_Y1, TOUCH_MAP_Y2, 0, lcd.height() - 1);
#else
    touch_last_x = map(p.x, TOUCH_MAP_X1, TOUCH_MAP_X2, 0, lcd.width() - 1);
    touch_last_y = map(p.y, TOUCH_MAP_Y1, TOUCH_MAP_Y2, 0, lcd.height() - 1);
#endif
    return true;
  }
  else
  {
    return false;
  }

#else
  return false;
#endif
}

bool touch_released()
{
#if defined(TOUCH_FT6X36)
  if (touch_released_flag)
  {
    touch_released_flag = false;
    return true;
  }
  else
  {
    return false;
  }

#elif defined(TOUCH_GT911)
  return true;

#elif defined(TOUCH_XPT2046)
  return true;

#else
  return false;
#endif
}
