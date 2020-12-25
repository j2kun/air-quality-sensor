#include <U8g2lib.h>
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
#define PLOT_START_X 5
#define PLOT_START_Y 65
#define PLOT_END_X 123
#define PLOT_END_Y 126
// The AQI value that will be at the top of the vertical axis
#define PLOT_RANGE 200

SFE_PARTICLE_SENSOR airSensor;
U8G2_SSD1327_MIDAS_128X128_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
CircularBuffer<int, PLOT_END_X - PLOT_START_X> recentAqis;

void setup() {
  // Wire.begin();
  if (u8g2.begin()) Serial.println("OLED Screen initialized...");
  if (airSensor.begin()) Serial.println("Particle Sensor initialized...");
  Serial.println("Exiting setup()");
}

void loop() {
  float pm25 = airSensor.getPM2_5();
  int aqi = computeAqi(pm25);

  if (recentAqis.isFull()) {
    // left end is oldest
    recentAqis.shift();
  }

  recentAqis.push(aqi);

  String aqiLabel = "AQI: " + (aqi == MAX_AQI ? "MAX" : String(aqi));
  String aqiBucketLabel = aqi <= 50 ? "Good"
    : aqi <= 100 ? "Moderate"
    : aqi <= 150 ? "Unhealthy"  // (sensitive groups)
    : aqi <= 200 ? "Unhealthy"
    : aqi <= 300 ? "Very Bad"
    : "Hazardous!";
  String sensitiveGroups = "(sensitive groups)";
  const unsigned char *aqiIcon = aqi <= 50 ? happy
    : aqi <= 100 ? neutral
    : aqi <= 150 ? sad
    : aqi <= 200 ? tongue
    : aqi <= 300 ? mask
    : skull;

  u8g2.firstPage();
  do {
    // Draw the header
    u8g2.drawLine(0, 16, SCREEN_WIDTH, 16);
    u8g2.setFont(u8g2_font_seraphimb1_tr);
    u8g2.drawStr(MID_SCREEN - u8g2.getStrWidth(HEADING) / 2, 13, HEADING);

    // Draw the icon
    u8g2.drawXBMP(10, 21, icon_width, icon_height, aqiIcon);

    // Draw the AQI label
    int textXPos = 10 + icon_width + 5;
    int aqiLabelYPos = 32;
    int textYSpace = 6;
    int fontHeight = 9;
    int bucketLabelYPos = aqiLabelYPos + fontHeight + textYSpace;
    int bucketLabelSubscriptYPos = bucketLabelYPos + textYSpace + 6;

    u8g2.drawStr(textXPos, aqiLabelYPos, aqiLabel.c_str());
    u8g2.drawStr(textXPos, bucketLabelYPos, aqiBucketLabel.c_str());

    // Smaller font needed to fit "sensitive groups" on the screen.
    u8g2.setFont(u8g2_font_finderskeepers_tf);
    if (aqi > 100 && aqi <= 150) {
        u8g2.drawStr(textXPos, bucketLabelSubscriptYPos, sensitiveGroups.c_str());
    }

    drawGraph();
  } while ( u8g2.nextPage() );

  delay(1000);
}

void drawGraph() {
    u8g2.setFont(u8g2_font_5x8_mn);

    // vertical axis
    u8g2.drawLine(PLOT_START_X, PLOT_START_Y, PLOT_START_X, PLOT_END_Y);
    // horizontal axis
    u8g2.drawLine(PLOT_START_X, PLOT_END_Y, PLOT_END_X, PLOT_END_Y);

    // Y axis top and middle ticks
    u8g2.drawStr(PLOT_START_X + 2, PLOT_START_Y + 4, String(PLOT_RANGE).c_str());
    u8g2.drawStr(PLOT_START_X + 2, PLOT_START_Y + 4 + (PLOT_END_Y - PLOT_START_Y) / 2, String(PLOT_RANGE / 2).c_str());

    int yPixel = 0, aqi = 0, nextYPixel = 0, x1 = 0, x2 = 0, y1 = 0, y2 = 0;
    for (char i = 0; i < recentAqis.size(); i++) {
        aqi = recentAqis[i];
        nextYPixel = map(aqi, 0, PLOT_RANGE, PLOT_END_Y, PLOT_START_Y);

        if (i == 0) {
            yPixel = nextYPixel;
        }

        x1 = PLOT_START_X + i;
        y1 = yPixel;
        x2 = PLOT_START_X + i + 1;
        y2 = nextYPixel;
        u8g2.drawLine(x1, y1, x2, y2);

        yPixel = nextYPixel;
    }

    // A floating y value tick that follows the last plotted value
    String aqiStr = String(aqi);
    int offset = u8g2.getStrWidth(aqiStr.c_str());
    int aqiLabelXPos = x2 + offset > 128 ? x2 - offset : x2;
    u8g2.drawStr(aqiLabelXPos, yPixel - 2, aqiStr.c_str());
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
