#include <Wire.h>
#include <WiFi.h>
#include <time.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C
#define OLED_RESET -1

// Pins from your wiring image
#define TOUCH_PIN 1
#define SDA_PIN   21
#define SCL_PIN   20

const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void drawEyes(bool closed) {
  if (closed) {
    display.fillRect(32, 30, 16, 4, SSD1306_WHITE);
    display.fillRect(80, 30, 16, 4, SSD1306_WHITE);
  } else {
    display.fillCircle(40, 32, 6, SSD1306_WHITE);
    display.fillCircle(88, 32, 6, SSD1306_WHITE);
  }
}

void drawMouth(bool happy) {
  if (happy) {
    display.drawFastHLine(60, 45, 8, SSD1306_WHITE);
    display.drawPixel(59, 44, SSD1306_WHITE);
    display.drawPixel(68, 44, SSD1306_WHITE);
  } else {
    display.drawFastHLine(56, 46, 16, SSD1306_WHITE);
  }
}

void showFace(bool blink = false, bool happy = false) {
  display.clearDisplay();
  drawEyes(blink);
  drawMouth(happy);
  display.display();
}

void showTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return;
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(34, 24);
  display.printf("%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
  display.display();
}

void setupTime() {
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 20) { delay(500); tries++; }
  configTime(20700, 0, "pool.ntp.org"); // Nepal Time (UTC+5:45)
  WiFi.disconnect(true);
}

void setup() {
  pinMode(TOUCH_PIN, INPUT);
  Wire.begin(SDA_PIN, SCL_PIN);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) while (true);
  setupTime();
  showFace();
}

void loop() {
  static unsigned long lastBlink = 0;
  static unsigned long touchStart = 0;
  bool touched = digitalRead(TOUCH_PIN);

  if (touched && touchStart == 0) touchStart = millis();
  if (!touched && touchStart > 0) {
    unsigned long pressTime = millis() - touchStart;
    touchStart = 0;
    if (pressTime < 500) { showFace(false, true); delay(1500); } 
    else { showTime(); delay(3000); }
    showFace();
  }

  if (millis() - lastBlink > random(3000, 6000)) {
    showFace(true, false); delay(120); showFace(false, false);
    lastBlink = millis();
  }
  delay(30);
}
