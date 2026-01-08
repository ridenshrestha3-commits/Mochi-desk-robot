#include <Wire.h>
#include <WiFi.h>
#include <time.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "MPU9250.h"

// Screen Settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C

// Pins from your Diagram
#define TOUCH_PIN 7
#define SDA_PIN   8
#define SCL_PIN   9

// WiFi Credentials
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// Objects
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
MPU9250 IMU(Wire, 0x68);

// Global Variables
unsigned long lastBlinkTime = 0;
unsigned long blinkDuration = 150;
bool isBlinking = false;
int eyeXOffset = 0;

void setup() {
  Serial.begin(115200);
  pinMode(TOUCH_PIN, INPUT);

  // Initialize I2C for ESP32-C3
  Wire.begin(SDA_PIN, SCL_PIN);

  // Initialize OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    for(;;); 
  }
  
  // Initialize MPU-9250
  IMU.begin();

  // Connect WiFi for Time
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(10, 25);
  display.print("Connecting WiFi...");
  display.display();

  WiFi.begin(ssid, password);
  int counter = 0;
  while (WiFi.status() != WL_CONNECTED && counter < 20) {
    delay(500);
    counter++;
  }

  // Set Time (Nepal UTC +5:45 = 20700 seconds)
  configTime(20700, 0, "pool.ntp.org");
}

void drawFace(int xShift, bool blink, bool happy) {
  display.clearDisplay();
  
  // Draw Eyes
  if (blink) {
    display.fillRect(30 + xShift, 30, 20, 4, WHITE);
    display.fillRect(78 + xShift, 30, 20, 4, WHITE);
  } else {
    display.fillCircle(40 + xShift, 32, 8, WHITE);
    display.fillCircle(88 + xShift, 32, 8, WHITE);
  }

  // Draw Mouth
  if (happy) {
    display.drawPixel(60, 48, WHITE);
    display.drawPixel(68, 48, WHITE);
    display.drawFastHLine(61, 49, 7, WHITE);
  } else {
    display.drawFastHLine(58, 50, 12, WHITE);
  }
  
  display.display();
}

void showTimeOverlay() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return;
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(35, 25);
  display.printf("%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
  display.display();
  delay(3000); // Show time for 3 seconds
}

void loop() {
  // 1. Check Motion (Tilt)
  IMU.readSensor();
  float ax = IMU.getAccelX_mss();
  eyeXOffset = map(ax * 10, -50, 50, -15, 15); // Eyes follow tilt

  // 2. Check Touch
  if (digitalRead(TOUCH_PIN) == HIGH) {
    showTimeOverlay();
  }

  // 3. Handle Blinking
  unsigned long currentMillis = millis();
  if (!isBlinking && (currentMillis - lastBlinkTime > random(3000, 8000))) {
    isBlinking = true;
    lastBlinkTime = currentMillis;
  }
  
  if (isBlinking && (currentMillis - lastBlinkTime > blinkDuration)) {
    isBlinking = false;
  }

  // 4. Update Face
  drawFace(eyeXOffset, isBlinking, false);
  
  delay(30);
}
