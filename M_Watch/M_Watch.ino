#include <Rtc_Pcf8563.h>

Rtc_Pcf8563 rtc;

#include "Wire.h"

#include <lm75.h>
float temperature;
float humidity;
TempI2C_LM75 termo = TempI2C_LM75(0x48,TempI2C_LM75::nine_bits);
#include <MPU6050.h>
MPU6050 mpu;
Vector rawGyro;
Vector normGyro;
float mpuscale = 1;
float mputemp;
#include "U8glib.h"
//-------字体设置，大、中、小
#define setFont_L u8g.setFont(u8g_font_7x13)
#define setFont_M u8g.setFont(u8g_font_fixed_v0r)
#define setFont_S u8g.setFont(u8g_font_fixed_v0r)

#define setFont_SS u8g.setFont(u8g_font_fub25n)
/*
font:
 u8g_font_7x13
 u8g_font_fixed_v0r
 u8g_font_chikitar
 u8g_font_osb21
 u8g_font_courB14r
 u8g_font_courB24n
 u8g_font_9x18Br
 */
//屏幕类型--------
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE);

#define init_draw 10  //主界面刷新时间

unsigned long timer_draw,timer;
int pkj=0;
String dateStr, ret;

void setup() {

  Serial.begin(115200);
  setTemp();
  setmpu6050();
  rtc.initClock();
//  setRtcTime(15, 5, 18, 1, 00, 00, 00);
  timer=millis();
}

void loop() {
  ret = getRtcTimeString();
  temperature=(float)termo.getTemp();
//  Serial.print(temperature);
//  Serial.println(" oC");
  rawGyro = mpu.readRawGyro();
  normGyro = mpu.readRawAccel();
  mputemp = mpu.readTemperature();
  
  if (millis() - timer_draw > init_draw)
  {
    pkj-=4;
    u8g.firstPage();
    do {
      draw();
    }
    while ( u8g.nextPage() );
    timer_draw = millis();
//    Serial.print(ret);
  }
}
DateTime mydate;
char str[16];  
//主界面，可自由定义
void draw()
{
  setFont_L;
  u8g.setPrintPos(0, 16);
//  u8g.print(rtc.formatDate());
//  u8g.print(rtc.formatDate(PCF_TIMEFORMAT_YYYY_MM_DD_H_M_S));
  mydate = rtc.getDateTime();
  sprintf(str,"%04u/%02u/%02u", mydate.year,mydate.month,mydate.day);  
  u8g.print(str);
  u8g.print(" ");
  switch (rtc.getWeekday()) {
    case 1:
      u8g.print("Mon");
      break;
    case 2:
      u8g.print("Tue");
      break;
    case 3:
      u8g.print("Wed");
      break;
    case 4:
      u8g.print("Thu");
      break;
    case 5:
      u8g.print("Fri");
      break;
    case 6:
      u8g.print("Sat");
      break;
    case 7:
      u8g.print("Sun");
      break;
  }
  u8g.setPrintPos(0, 64);
  u8g.print("T:");
  u8g.print(temperature,1);
  u8g.print("`c ");
  u8g.print(mputemp,1);
  u8g.print("`c ");

u8g.setPrintPos(100, 49);
//sprintf(str,"%f", (float)normGyro.XAxis/mpuscale);  
dtostrf((float)normGyro.XAxis/mpuscale,4, 1, str);
u8g.print(str);
u8g.setPrintPos(100, 49-15);
dtostrf((float)normGyro.YAxis/mpuscale,4, 1, str);
u8g.print(str);
u8g.setPrintPos(100, 49-30);
dtostrf((float)normGyro.ZAxis/mpuscale,4, 1, str);
u8g.print(str);

  
//  u8g.setPrintPos(pkj, 64); 
//  u8g.print("Welcome! www.microduino.ccC:UserscaojiDocumentsArdu");
  setFont_SS;
  u8g.setPrintPos(18-10, 49);
  if (rtc.getHour() < 10)
  {
    u8g.print("0");
    u8g.print(rtc.getHour());
  }
  else
    u8g.print(rtc.getHour());
  u8g.setPrintPos(55-10, 46);
  if (rtc.getSecond() % 2 == 0)
    u8g.print(":");
  else
    u8g.print(" ");
  u8g.setPrintPos(68-10, 48);
  if (rtc.getMinute() < 10)
  {
    u8g.print("0");
    u8g.print(rtc.getMinute());
  }
  else
    u8g.print(rtc.getMinute());
}

void setRtcTime (byte _year, byte _month, byte _day, byte _week, byte _hour, byte _minute, byte _sec)
{
  //clear out all the registers
  
  rtc.setDate(_day, _week, _month, 0, _year);
  rtc.setTime(_hour, _minute, _sec);
}

String getRtcTimeString() {
  dateStr = rtc.formatDate();
  dateStr += " ";
  dateStr += rtc.formatTime();

  return dateStr;
}
void setTemp()
{
  //termo.setResolution(TempI2C_LM75::ten_bits);
//termo.setResolution(TempI2C_LM75::eleven_bits);
termo.setResolution(TempI2C_LM75::twelve_bits);
//termo.setResolution(TempI2C_LM75::nine_bits);
termo.setTHyst(25);
termo.setTOS(120);
termo.setTermostatMode(TempI2C_LM75::interrupt_mode);
termo.setTermostatMode(TempI2C_LM75::comparator_mode);
termo.setTermostatFaultTolerance(TempI2C_LM75::four_samples);
termo.setTermostatFaultTolerance(TempI2C_LM75::one_samples);
termo.setOSPolarity(TempI2C_LM75::active_high);
termo.setOSPolarity(TempI2C_LM75::active_low);};
void setmpu6050()
{
  while(!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G)) 
{    Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
     delay(500); 
}

mpu.calibrateGyro();

mpu.setThreshold(3);
checkSettings();
}
void checkSettings()
{
  Serial.println();
  
  Serial.print(" * Sleep Mode:        ");
  Serial.println(mpu.getSleepEnabled() ? "Enabled" : "Disabled");
  
  Serial.print(" * Clock Source:      ");
  switch(mpu.getClockSource())
  {
    case MPU6050_CLOCK_KEEP_RESET:     Serial.println("Stops the clock and keeps the timing generator in reset"); break;
    case MPU6050_CLOCK_EXTERNAL_19MHZ: Serial.println("PLL with external 19.2MHz reference"); break;
    case MPU6050_CLOCK_EXTERNAL_32KHZ: Serial.println("PLL with external 32.768kHz reference"); break;
    case MPU6050_CLOCK_PLL_ZGYRO:      Serial.println("PLL with Z axis gyroscope reference"); break;
    case MPU6050_CLOCK_PLL_YGYRO:      Serial.println("PLL with Y axis gyroscope reference"); break;
    case MPU6050_CLOCK_PLL_XGYRO:      Serial.println("PLL with X axis gyroscope reference"); break;
    case MPU6050_CLOCK_INTERNAL_8MHZ:  Serial.println("Internal 8MHz oscillator"); break;
  }
  
  Serial.print(" * Gyroscope:         ");
  switch(mpu.getScale())
  {
    case MPU6050_SCALE_2000DPS:        Serial.println("2000 dps"); mpuscale = 2000; break;
    case MPU6050_SCALE_1000DPS:        Serial.println("1000 dps"); mpuscale = 1000; break;
    case MPU6050_SCALE_500DPS:         Serial.println("500 dps"); mpuscale = 500; break;
    case MPU6050_SCALE_250DPS:         Serial.println("250 dps"); mpuscale = 250; break;
  } 
  
  Serial.print(" * Gyroscope offsets: ");
  Serial.print(mpu.getGyroOffsetX());
  Serial.print(" / ");
  Serial.print(mpu.getGyroOffsetY());
  Serial.print(" / ");
  Serial.println(mpu.getGyroOffsetZ());
  
  Serial.println();
}
