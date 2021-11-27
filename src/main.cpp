#include <Arduino.h>
#include <Wire.h>
#include <SH1106Wire.h>
#include <DHT.h> // Digital relative humidity & temperature sensor AM2302/DHT22
#include "SlopeTracker.h"
// #include "DFRobot_LCD.h"


#define TIME_STEP_DISP 1000
//
//#define DHTTYPE DHT11
//#define DHTPIN 19

#define DHTTYPE DHT22 // DHT 22 (AM2302)
#define DHTPIN 23
DHT dht(DHTPIN, DHTTYPE);

// DFRobot_LCD lcd(16, 2);
SH1106Wire lcd(0x3c, SDA, SCL); // ADDRESS, SDA, SCL

const int x_pin = 34,
          button_pin = 35;
unsigned long int time_next_d = 0;
float temp_realtime, rh_realtime, rh_1s_mva, rh_rate;
const uint8_t avg_sample_time = 250;                          // ms
const uint16_t trend_sample_time = 1000;                      // ms
SlopeTracker rh_short_buffer(4, avg_sample_time / 60000.0);   // 4 data points, 0.25 second each
SlopeTracker rh_long_buffer(30, trend_sample_time / 60000.0); // 60 data points, 0.5 minutes each
unsigned long avg_timer_due = 0, trend_timer_due = 0;
uint8_t x_1col = 46, x_2col = 100, y_1row = 12, y_2row = 32;

void draw_background()
{
  lcd.setFont(ArialMT_Plain_16);
  lcd.setTextAlignment(TEXT_ALIGN_RIGHT);
  lcd.drawString(x_1col, y_1row, " RH=");
  lcd.drawString(x_1col, y_2row, "dRH=");
  lcd.setFont(ArialMT_Plain_10);
  lcd.setTextAlignment(TEXT_ALIGN_LEFT);
  lcd.drawString(x_2col, y_1row, "%");
  lcd.drawString(x_2col, y_2row, "%/Min");
}

void setup()
{
  Serial.begin(9600);
  lcd.init();
  lcd.flipScreenVertically();
  // lcd.setColor(WHITE);
  lcd.clear();
  draw_background();
  pinMode(x_pin, INPUT);
  pinMode(button_pin, INPUT);
  // lcd.displayOn();
  lcd.setTextAlignment(TEXT_ALIGN_LEFT);
  lcd.display();

  dht.begin();
}

void loop()
{
  temp_realtime = dht.readTemperature(); // read_sensor
  rh_realtime = dht.readHumidity();      // read_sensor

  if (millis() >= avg_timer_due)
  {
    avg_timer_due += avg_sample_time;
    rh_short_buffer.addPoint(rh_realtime);
    if (rh_short_buffer.ready())
    {
      rh_1s_mva = rh_short_buffer.getAvg();
    }
    else
    {
      rh_1s_mva = rh_realtime;
    }
  }
  if (millis() >= trend_timer_due)
  {
    trend_timer_due += trend_sample_time;
    rh_long_buffer.addPoint(rh_1s_mva);
    if (rh_long_buffer.ready())
    {
      rh_rate = rh_long_buffer.getSlope();
    }
  }

  /*
    if (!digitalRead(button_pin))
    {
      Serial.println("Reseted Buffer");
      rh_long_buffer.reset();
    }
  */

  if (millis() > time_next_d)
  {
    time_next_d += TIME_STEP_DISP;
    uint16_t xc = 48;
    lcd.clear();
    draw_background();
    lcd.setFont(ArialMT_Plain_16);
    lcd.drawString(xc, y_1row, "     ");
    lcd.drawString(xc, y_1row, String(rh_realtime));
    if (rh_long_buffer.ready())
    {
      lcd.drawString(xc, y_2row, "    ");
      lcd.drawString(xc, y_2row, String(rh_rate));
      lcd.print(rh_rate);
      xc = 64;
      /*
            float thr1 = 0.08, thr2 = 0.5;
            if (rh_rate < -thr2)
              lcd.drawString(xc, y_1row, "<<");
            else if (rh_rate > thr2)
              lcd.drawString(xc, y_1row, ">>");
            else if (rh_rate < -thr1)
              lcd.drawString(xc, y_1row, "<");
            else if (rh_rate > thr1)
              lcd.drawString(xc, y_1row, ">");
            else
              lcd.drawString(xc, y_1row, "-");
              */
    }
    lcd.display();
  }
}