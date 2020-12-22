#include <Wire.h>
#include <U8g2lib.h>
#include <SparkFunTMP102.h>
#include <SparkFun_Particle_Sensor_SN-GCJA5_Arduino_Library.h>
#include <CircularBuffer.h>
 
// Icons defined as XBM data arrays
#include "image.h"

#define HEADING "Air Quality Index"
#define MAX_AQI 501
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128
#define MID_SCREEN 64

// The sreen is 128x128 pixels, with the origin in the top left. These macros
// allow you to change the placement and scaling of the chart
#define PLOT_START_X 10
#define PLOT_START_Y 20
#define PLOT_END_X 118
#define PLOT_END_Y 50
// The AQI value that will be at the top of the vertical axis
#define PLOT_RANGE 100

SFE_PARTICLE_SENSOR airSensor;
U8G2_SSD1327_MIDAS_128X128_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
CircularBuffer<int, 100> recentAqis;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  if (u8g2.begin()) Serial.println("OLED Screen initialized...");
  if (airSensor.begin()) Serial.println("Particle Sensor initialized...");
  Serial.println("Exiting setup()");
}

void loop() {
  float pm25 = airSensor.getPM2_5();
  float aqi = computeAqi(pm25);

  if (recentAqis.isFull()) {
    // left end is oldest
    recentAqis.shift();
  }

  recentAqis.push(aqi);

  String aqiLabel = "AQI:   " + (aqi == MAX_AQI ? "MAX" : String(aqi));
  String aqiBucketLabel = aqi <= 50 ? "Good"
    : aqi <= 100 ? "Moderate"
    : aqi <= 150 ? "USG"
    : aqi <= 200 ? "Unhealthy"
    : aqi <= 300 ? "Very Unhealthy"
    : "Hazardous!";
  const unsigned char *aqiIcon = aqi <= 50 ? happy
    : aqi <= 100 ? neutral
    : aqi <= 150 ? sad
    : aqi <= 200 ? tongue
    : aqi <= 300 ? mask
    : skull;

  u8g2.firstPage();
  do {
    u8g2.drawRFrame(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 8);
    u8g2.drawXBMP(50, 76, icon_width, icon_height, aqiIcon);
    u8g2.setFont(u8g2_font_tenthinguys_tr);
    u8g2.drawStr(MID_SCREEN - u8g2.getStrWidth(HEADING) / 2, 13, HEADING);
    u8g2.drawStr(MID_SCREEN - u8g2.getStrWidth(aqiBucketLabel.c_str()) / 2, 123, aqiBucketLabel.c_str());
    u8g2.drawLine(0, 16, SCREEN_WIDTH, 16);
    u8g2.drawLine(0, 110, SCREEN_WIDTH, 110);
    u8g2.setFont(u8g2_font_9x15_mr);
    u8g2.drawStr(4, 67, aqiLabel.c_str());

    drawGraph();
  } while ( u8g2.nextPage() );

  // To avoid banging on the I2C sensors
  delay(1000);
}

void drawGraph() {
    // vertical axis
    u8g2.drawLine(PLOT_START_X, PLOT_START_Y, PLOT_START_X, PLOT_END_Y);
    // horizontal axis
    u8g2.drawLine(PLOT_START_X, PLOT_END_Y, PLOT_END_X, PLOT_END_Y);

    int yPixel = 0;
    for (char i = 0; i < recentAqis.size(); i++) {
        int aqi = recentAqis[i];
        float yPercent = (PLOT_RANGE - (float) aqi) / PLOT_RANGE;
        int nextYPixel = (int) (yPercent * (PLOT_END_Y - PLOT_START_Y)) + PLOT_START_Y;

        if (i == 0) {
            yPixel = nextYPixel;
        }

        int x1 = PLOT_START_X + i;
        int y1 = yPixel;
        int x2 = PLOT_START_X + i + 1;
        int y2 = nextYPixel;
        u8g2.drawLine(x1, y1, x2, y2);

        yPixel = nextYPixel;
    }
}

int computeAqi(float pm) {
  if (pm > 500.4) {
    return MAX_AQI;
  }

  float lowerPms[] = {0, 12.1, 35.5, 55.5, 150.5, 250.5, 350.5, 500.4};
  int lowerAqis[] = {0, 51, 101, 151, 201, 301, 401, 500};
  float upperPm = -1.0, lowerPm = 0.0;
  int lowerAqi = 0, upperAqi = 0;
  int i = 0;
  while (pm > upperPm && i < 7) {
    lowerPm = lowerPms[i];
    upperPm = lowerPms[i + 1];
    lowerAqi = lowerAqis[i];
    upperAqi = lowerAqis[i + 1];
    i++;
  }
  return (int) (((float) upperAqi - lowerAqi) / (upperPm - lowerPm) * (pm - lowerPm) + lowerAqi);
}
