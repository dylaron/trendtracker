#include <Arduino.h>
#include <Wire.h>
#include <SH1106Wire.h>
#include <DHT.h> // Digital relative humidity & temperature sensor AM2302/DHT22
#include "SlopeTracker.h"

#define TIME_STEP_DISP 1000
//
//#define DHTTYPE DHT11
//#define DHTPIN 19

#define DHTTYPE DHT22 // DHT 22 (AM2302)
#define DHTPIN 23
DHT dht(DHTPIN, DHTTYPE);

SH1106Wire display(0x3c, SDA, SCL); // ADDRESS, SDA, SCL

const int x_pin = 34,
          button_pin = 35;
unsigned long int time_next_d = 0;
float temp_realtime, rh_realtime, rh_1s_mva, rh_rate;
const uint8_t avg_sample_time = 250;                          // ms
const uint16_t trend_sample_time = 1000;                      // ms
SlopeTracker rh_short_buffer(4, avg_sample_time / 60000.0);   // 4 data points, 0.25 second each
SlopeTracker rh_long_buffer(30, trend_sample_time / 60000.0); // 60 data points, 0.5 minutes each
unsigned long avg_timer_due = 0, trend_timer_due = 0;
uint8_t x_1col = 46, x_2col = 100, y_1row = 8, y_2row = 26, y_3row = 44;

void draw_background()
{
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(x_1col, y_1row, "Tmp=");
  display.drawString(x_1col, y_2row, " RH=");
  display.drawString(x_1col, y_3row, "dRH=");
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(x_2col, y_1row, "'C");
  display.drawString(x_2col, y_2row, "%");
  display.drawString(x_2col, y_3row, "%/Min");
}

void setup()
{
  Serial.begin(9600);
  display.init();
  display.flipScreenVertically();
  // display.setColor(WHITE);
  display.clear();
  draw_background();
  pinMode(x_pin, INPUT);
  pinMode(button_pin, INPUT);
  // display.displayOn();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.display();

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
    uint16_t xc = x_1col + 2;
    display.clear();
    draw_background();
    display.setFont(ArialMT_Plain_16);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(xc, y_1row, String(temp_realtime));
    display.drawString(xc, y_2row, String(rh_1s_mva));
    if (rh_long_buffer.ready())
    {
      display.drawString(xc, y_3row, String(rh_rate));
      display.print(rh_rate);
      xc = x_2col + 10;

      float thr1 = 0.08, thr2 = 0.5;
      if (rh_rate < -thr2)
        display.drawString(xc, y_2row, "<<");
      else if (rh_rate > thr2)
        display.drawString(xc, y_2row, ">>");
      else if (rh_rate < -thr1)
        display.drawString(xc, y_2row, "<");
      else if (rh_rate > thr1)
        display.drawString(xc, y_2row, ">");
      else
        display.drawString(xc, y_2row, "-");
    }
    display.display();
  }
}