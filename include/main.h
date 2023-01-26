#pragma once
#include "DeepPass.h"
#include <Arduino.h>
#include <EEPROM.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <ArduinoOTA.h>
#include <littleFS.h>

// Filesystem

#define EEPROM_LEVEL_X 0
#define EEPROM_LEVEL_Y 2
#define EEPROM_VALUATION_X 4
#define EEPROM_VALUATION_Y 6
#define EEPROM_LEVEL_THRESHOLD 8
#define EEPROM_LEVEL_INVERTATION 9
#define EEPROM_LEVEL_ACCESSPOINT 10

// Webserver

extern const char *ssid;
extern const char *password;

extern WebServer webServer;

const byte DNS_PORT = 53;
extern DNSServer dnsServer;
extern Adafruit_ADXL345_Unified accel;

// Over the Air Update

extern const char deviceName[];

extern bool accelInitialized;
extern int levelX;
extern int levelY;
extern int calibrationX;
extern int calibrationY;
extern int valuationX;
extern int valuationY;
extern bool valuationActive;
extern uint8_t levelThreshold;
extern bool invertAxis;
extern bool useAccessPointMode;
extern long lastMillis;
extern long lastMillisClientAvailable;
extern int minValuationX;
extern int minValuationY;
extern int maxValuationX;
extern int maxValuationY;
extern DeepPass<int> deepPassX;
extern DeepPass<int> deepPassY;


class Level
{
public:
    void getLevel();
    void CalibrateLevel();

private:
};

extern Level level;

class Helper
{
public:
    void SerialBegin();
    void Adxl345Begin();
    void LittleFSBegin();
    void EepromBegin();
    boolean isIp(String str);
    bool ProcessETag(const char *ETag);
    void ProcessSetupArguments();
    String toStringIp(IPAddress ip);
    void writeIntIntoEEPROM(int address, int16_t number);
    int16_t readIntFromEEPROM(int address);
    String GetCustomText();
};

extern Helper helper;

class OTA
{
public:
    void setupOTA();
};

extern OTA ota;

class Web_Server
{
public:
    void WiFiBegin();
    void ConnectToAccessPoint();
    void CreateAccessPoint();
    static void handle_root();
    static void handle_level();
    static void handle_setup();
    static void handle_calibrate();
    static void handle_valuation();
    static void handle_reset();
    static void handleNotFound();
    static void handle_success();
    boolean captivePortal();
    static void redirect();
    static void PrintIncomingRequest();
    static String getContentType(String filename);
    static void handleFileRead();
    static void handle_fileupload();
};

extern Web_Server webserver;

class Data
{
public:
    void LoadData();
    void LoadLevel();
    void LoadValuation();
    void LoadLevelThreshold();
    void LoadInvertation();
    void LoadAP();
    void StoreLevel();
    void StoreLevelValuation();
    void StoreLevelThreshold();
    void StoreInvertation();
    void StoreAP();
    void Valuation();
    void InitDeepPass();
};

extern Data data;
