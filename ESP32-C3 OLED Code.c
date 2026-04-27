#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>
#include "esp_sleep.h"

#define SDA_PIN 5
#define SCL_PIN 6

// ===================== SETTINGS =====================
#define UPDATE_INTERVAL_MS 4000 // sensor + display update rate
#define PRESSURE_WINDOW_MS 7200000 // 2 hour worth of pressure data to keep
#define TREND_WINDOW_MS 3600000 // 60 minutes minimum pressure data before predicting weather
#define FAST_TREND_WINDOW_MS 900000   // 15 minutes to detect sudden pressure changes
// Power modes: 
// 0 = always on (no sleep) 
// 1 = light sleep between updates !!!!!!!!!!!!!!!!!!!!!!!!!(after flashing with this, future flashes you have to flash by pressing BOOT button due to cpu resets) !!!!!!!!!!!!!!!!!!!!!!!!!!
// 2 = deep sleep between updates (buggy)
#define POWER_MODE 1
// ====================================================

U8G2_SSD1306_72X40_ER_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

Adafruit_AHTX0 aht;
Adafruit_BMP280 bmp;

float tempF = 0;
float humidity = 0;
float pressure = 0;

//Weather Prediction code
#define PRESSURE_SAMPLES (PRESSURE_WINDOW_MS / UPDATE_INTERVAL_MS)
#define TREND_SAMPLES (TREND_WINDOW_MS / UPDATE_INTERVAL_MS)
#define FAST_TREND_SAMPLES (FAST_TREND_WINDOW_MS / UPDATE_INTERVAL_MS)
int requiredSamples = TREND_SAMPLES;
float pressureHistory[PRESSURE_SAMPLES];
int pressureIndex = 0;
bool pressureFilled = false;
static float smoothPressure = 0;

unsigned long lastUpdate = 0;
bool toggle = false;

// ---------------- SENSOR READ ----------------
void readSensors() {
  sensors_event_t h, t;
  aht.getEvent(&h, &t);

  if (!isnan(h.relative_humidity) && !isnan(t.temperature)) {
    humidity = constrain(h.relative_humidity, 0, 100);
    tempF = t.temperature * 9.0 / 5.0 + 32.0;
  }

  float rawPressure = bmp.readPressure() / 100.0;

  // simple smoothing (low-pass filter)
  if (smoothPressure == 0) smoothPressure = rawPressure;
  smoothPressure = 0.9 * smoothPressure + 0.1 * rawPressure;
  pressure = smoothPressure;

  pressureHistory[pressureIndex] = pressure;
  pressureIndex = (pressureIndex + 1) % PRESSURE_SAMPLES;
  if (pressureIndex == 0) pressureFilled = true;
}

float getPressureTrend() {
  int samples = pressureFilled ? PRESSURE_SAMPLES : pressureIndex;
  if (samples < requiredSamples) return 0;

  int oldIndex = (pressureIndex - requiredSamples + PRESSURE_SAMPLES) % PRESSURE_SAMPLES;
  int newIndex = (pressureIndex - 1 + PRESSURE_SAMPLES) % PRESSURE_SAMPLES;

  float oldP = pressureHistory[oldIndex];
  float newP = pressureHistory[newIndex];

  return newP - oldP;
}

float getFastPressureTrend() {
  int samples = pressureFilled ? PRESSURE_SAMPLES : pressureIndex;
  if (samples < FAST_TREND_SAMPLES) return 0;

  int oldIndex = (pressureIndex - FAST_TREND_SAMPLES + PRESSURE_SAMPLES) % PRESSURE_SAMPLES;
  int newIndex = (pressureIndex - 1 + PRESSURE_SAMPLES) % PRESSURE_SAMPLES;

  float oldP = pressureHistory[oldIndex];
  float newP = pressureHistory[newIndex];

  return newP - oldP;
}

// ---------------- DISPLAY ----------------
void drawScreen() {
  u8g2.clearBuffer();

  const int iconX = 0;
  const int textX = 11;   // space for icon
  const int line1Y = 16;
  const int line2Y = 36;

  // =========================
  // LINE 1: TEMP
  // =========================
  u8g2.setFont(u8g2_font_open_iconic_weather_2x_t);
  //Draw thermometer
  int x=4; int y=13;
  u8g2.drawCircle(x,y,3); // bulb
  u8g2.drawLine(x-1,y-2,x-1,y-12); u8g2.drawLine(x+1,y-2,x+1,y-12); // tube
  u8g2.drawBox(x,y-13,1,3); // seal
  //u8g2.drawLine(x,y-10,x,y-7); // mercury level

  u8g2.setFont(u8g2_font_logisoso16_tr);
  u8g2.setCursor(textX, line1Y);
  u8g2.print(tempF, 1);
  u8g2.print("F");

  // =========================
  // LINE 2: HUM / PRESS
  // =========================
  toggle = !toggle;

  if (toggle) {
    u8g2.setFont(u8g2_font_open_iconic_weather_2x_t);

    //Water droplet icon
    int x=4; int y=30;
    u8g2.drawDisc(x,y,4); // filled bulb (wider)
    u8g2.drawTriangle(x,y-10,x-5,y-2,x+5,y-2); // main body

    //Print humidity
    u8g2.setFont(u8g2_font_logisoso16_tr);
    u8g2.setCursor(textX+3, line2Y);
    u8g2.print(humidity, 0);
    u8g2.print("%");

  }
  else {
    float trend = getPressureTrend();
    float fastTrend = getFastPressureTrend();

    u8g2.setFont(u8g2_font_open_iconic_weather_2x_t);
    if (!pressureFilled && pressureIndex < requiredSamples) { 
      u8g2.setFont(u8g2_font_7x13_tr);
      u8g2.setCursor(iconX, line2Y);
      u8g2.print("Pres");
    }
    else if (fastTrend < -0.3) {
      u8g2.drawGlyph(iconX+5, line2Y, 67); // strong warning (rain/unstable)
    }
    else if (trend < -0.8) {
      u8g2.drawGlyph(iconX+5, line2Y, 67); // rain-ish 67 / unstable
    } 
    else if (trend > 0.8) {
      u8g2.drawGlyph(iconX+5, line2Y, 70); // sun 70 / improving
    } 
    else {
      u8g2.setFont(u8g2_font_6x10_tr);
      u8g2.setCursor(iconX, line2Y);
      u8g2.print("pStbl");
    }

    u8g2.setFont(u8g2_font_logisoso16_tr);
    u8g2.setCursor(textX+18, line2Y);
    u8g2.print(pressure, 0);

    if (!pressureFilled){ //Will show a . in bottom right corner, when it dissapears you know you have 1hr + of samples
      u8g2.setFont(u8g2_font_6x10_tr);
      u8g2.setCursor(textX+59, line2Y);
      u8g2.print(".");     
    }
  }

  u8g2.sendBuffer();

}

// ---------------- SLEEP ----------------
void lightSleep(uint32_t ms) {
#if POWER_MODE == 1
  esp_sleep_enable_timer_wakeup(ms * 1000ULL);
  esp_light_sleep_start();
#endif
}

void deepSleep(uint32_t ms) {
#if POWER_MODE == 2
  esp_sleep_enable_timer_wakeup(ms * 1000ULL);
  esp_deep_sleep_start();
#endif
}

// ---------------- SETUP ----------------
void setup() {
  Wire.begin(SDA_PIN, SCL_PIN);

  u8g2.begin();

  aht.begin(&Wire);

  if (!bmp.begin(0x76)) {
    bmp.begin(0x77);
  }
  delay(1000);
}

// ---------------- LOOP ----------------
void loop() {

#if POWER_MODE == 0
  if (millis() - lastUpdate > UPDATE_INTERVAL_MS) {
    lastUpdate = millis();
    readSensors();
    drawScreen();
  }
#else
  readSensors();
  drawScreen();

  lightSleep(UPDATE_INTERVAL_MS);
#endif
}
