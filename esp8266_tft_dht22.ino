#include <TFT_eSPI.h>
#include "Font.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
//#define DHTTYPE    DHT11     // DHT 11
#define DHTTYPE    DHT22     // DHT 22 (AM2302)
#define DHTPIN  5
#define TFT_BG  4
DHT_Unified dht(DHTPIN, DHTTYPE);
TFT_eSPI tft = TFT_eSPI();
void setup() {

  Serial.begin(115200);
  pinMode(TFT_BG, OUTPUT); //背光
  digitalWrite(TFT_BG, HIGH);
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_WHITE);
  dht.begin();

  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS initialze error");
    while (1) yield(); // wait
  }

  drawBmp("/temperature.bmp", 5, 40);
  drawText("室内温度：", 65, 50);
  drawBmp("/humidity.bmp", 5, 120);
  drawText("室内湿度：", 65, 130);
}

void loop() {

  delay(5000);

  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
  }
  else {
    Serial.print(F("Temperature: "));
    Serial.print(event.temperature);
    Serial.println(F("°C"));
    tft.drawNumber(event.temperature, 255, 48, 6);
  }

  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
  }
  else {
    Serial.print(F("Humidity: "));
    Serial.print(event.relative_humidity);
    Serial.println(F("%"));
    tft.drawNumber(event.relative_humidity, 255, 128, 6);
  }

}


void drawText(String text, int x, int y) {
  //  tft.setTextColor(TFT_WHITE, TFT_BLACK); //白色文字
  tft.setTextColor(TFT_BLACK, TFT_WHITE);//黑色文字
  tft.loadFont(DHTFont);
  tft.setCursor(x, y);
  tft.println(text);
  tft.unloadFont();
}
void drawBmp(String filename, int16_t x, int16_t y) {

  if ((x >= tft.width()) || (y >= tft.height())) return;

  fs::File bmpFS;

  // Open requested file on SD card
  bmpFS = SPIFFS.open(filename, "r");

  if (!bmpFS)
  {
    Serial.print(filename);
    Serial.print("File not found");
    return;
  }

  uint32_t seekOffset;
  uint16_t w, h, row, col;
  uint8_t  r, g, b;

  uint32_t startTime = millis();

  if (read16(bmpFS) == 0x4D42)
  {
    read32(bmpFS);
    read32(bmpFS);
    seekOffset = read32(bmpFS);
    read32(bmpFS);
    w = read32(bmpFS);
    h = read32(bmpFS);

    if ((read16(bmpFS) == 1) && (read16(bmpFS) == 24) && (read32(bmpFS) == 0))
    {
      y += h - 1;

      bool oldSwapBytes = tft.getSwapBytes();
      tft.setSwapBytes(true);
      bmpFS.seek(seekOffset);

      uint16_t padding = (4 - ((w * 3) & 3)) & 3;
      uint8_t lineBuffer[w * 3 + padding];

      for (row = 0; row < h; row++) {

        bmpFS.read(lineBuffer, sizeof(lineBuffer));
        uint8_t*  bptr = lineBuffer;
        uint16_t* tptr = (uint16_t*)lineBuffer;
        // Convert 24 to 16 bit colours
        for (uint16_t col = 0; col < w; col++)
        {
          b = *bptr++;
          g = *bptr++;
          r = *bptr++;
          *tptr++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
        }

        // Push the pixel row to screen, pushImage will crop the line if needed
        // y is decremented as the BMP image is drawn bottom up
        tft.pushImage(x, y--, w, 1, (uint16_t*)lineBuffer);
      }
      tft.setSwapBytes(oldSwapBytes);
      // Serial.print("Loaded in "); Serial.print(millis() - startTime);
      // Serial.println(" ms");
    }
    else Serial.println("BMP format not recognized.");
  }
  bmpFS.close();
}

uint16_t read16(fs::File &f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(fs::File &f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}
