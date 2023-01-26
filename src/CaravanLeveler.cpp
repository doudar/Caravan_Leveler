// Gyro ADXL345
#include "main.h"

const char deviceName[] = "Sport&Fun Leveler";
const char *ssid = "FindMichDoch2";
const char *password = "5AF714A8B";
bool accelInitialized = false;
int levelX = 0;
int levelY = 0;
int calibrationX = -150;
int calibrationY = -25;
int valuationX = 271;
int valuationY = 267;
bool valuationActive = false;
uint8_t levelThreshold = 5;
bool invertAxis = false;
bool useAcessPointMode = false;
long lastMillis = 0;
long lastMillisClientAvailable = 0;
DeepPass<int> deepPassX(1.0 / 5.0);
DeepPass<int> deepPassY(1.0 / 5.0);
/* Assign a unique ID to this sensor at the same time */
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);
WebServer webServer(80);
Helper helper;
Data data;
Web_Server webserver;
OTA ota;
Level level;
DNSServer dnsServer;
void setup()
{
  helper.SerialBegin();
  helper.Adxl345Begin();
  helper.EepromBegin();

  data.LoadData();

  helper.LittleFSBegin();
  webserver.WiFiBegin();
  ota.setupOTA();
}

void loop()
{
  // WebServer
  webServer.handleClient();

  // DNS
  dnsServer.processNextRequest();

  // OTA
  ArduinoOTA.handle();

  if (millis() - lastMillis > 200)
  {
    // Only update when someone is listening:
    if (millis() - lastMillisClientAvailable < 1000)
      level.getLevel();
    lastMillis = millis();
    Serial.print(".");
  }
}
