#ifdef __IN_ECLIPSE__
//This is a automatic generated file
//Please do not modify this file
//If you touch this file your change will be overwritten during the next build
//This file has been generated on 2018-07-02 18:14:29

#include "Arduino.h"
#include "Arduino.h"
#include "libraries/WiFiManager/src/WiFiManager.h"
#include "libraries/NTPClient.h"
#include "libraries/Timezone.h"
#include <Update.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <HTTPClient.h>
#include <Wire.h>
#include "libraries/OLED/SSD1306.h"
#include "libraries/OLED/OLEDDisplayUi.h"
#include "images.h"
extern WebServer server;
#include <WiFiClient.h>
#include <DNSServer.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>
#define LED0_PIN 16
#define CONFIG_WIFI_PIN 27
#define INPUT1_PIN 14
#define OUTPUT0_PIN 32
#define OUTPUT1_PIN 33
#define OUTPUT2_PIN 25
#define OUTPUT3_PIN 26
extern WiFiUDP ntpUDP;
extern NTPClient timeClient;
extern int reconnectTimeout;
extern TimeChangeRule CEST;
extern TimeChangeRule CET;
extern Timezone CE;
#define DEVICES_NUM 8
#define OUTPUT_BIT 0
#define MANUAL_BIT 1
#define CMD_BIT 2
#define UNACK_BIT 3
#define RUNONCE_BIT 4
#define SAMPLES 16
extern float level;
extern unsigned long valveOpenSecCounters[];
extern IPAddress deviceIP;
extern bool isAP;
extern bool checkin;
#define ARDUINO_RUNNING_CORE 1
extern bool errorConn;
#define OLED_ADDRESS 0x3c
#define OLED_SDA 5
#define OLED_SCL 4
extern SSD1306 display;
#define X2 128
extern FrameCallback frames[];
extern int frameCount;
extern OverlayCallback overlays[];
extern int overlaysCount;
extern int lastFrameNo;
extern String lastMessage;
extern const char* host;
extern const char* update_path;
extern char* htmlHeader;
extern char* htmlFooter;
extern const char* serverIndex;
extern const char* www_username;
extern char www_password[];
extern char serverName[];
extern char writeApiKey[];
extern unsigned int talkbackID;
extern char talkbackApiKey[];
extern HTTPClient http;
extern int httpCode;
extern unsigned int errorCounter;
extern unsigned int r1Off;
extern unsigned int r1On;
extern unsigned int r2Off;
extern unsigned int r2On;
extern unsigned int r3Off;
extern unsigned int r3On;
extern unsigned int r4Off;
extern unsigned int r4On;
extern unsigned int rAllOff;
extern unsigned int rAllOn;
#include "libraries/RCSwitch.h"
extern RCSwitch rcSwitch;
#include "libraries/interval.h"

float analogRead(int pin, int samples) ;
void msOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) ;
void drawFrame1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) ;
void drawFrameA1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) ;
String deviceToString(struct Device device);
void drawFrameD1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) ;
void drawFrameD2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) ;
void drawFrameM1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) ;
void drawDisplay(OLEDDisplay *display) ;
void drawDisplay(OLEDDisplay *display, int frame) ;
void drawMessage(OLEDDisplay *display, String msg) ;
String getDeviceForm(int i, struct Device devices[]) ;
void handleRoot();
void saveApi() ;
void saveInstruments() ;
void startWiFiAP() ;
void setup() ;
void loop1(void *pvParameters) ;
void loop() ;

#include "eCan.ino"


#endif
