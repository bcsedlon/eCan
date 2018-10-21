#ifdef __IN_ECLIPSE__
//This is a automatic generated file
//Please do not modify this file
//If you touch this file your change will be overwritten during the next build
//This file has been generated on 2018-10-21 16:15:28

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
extern WebServer httpServer;
#define DRAWMESSAGE(display, message) (drawMessage(&display, message))
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <DNSServer.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>
#include <FS.h>
#include "libraries/SD/src/SD.h"
#include <SPI.h>
#define LED0_PIN 16
#define CONFIG_WIFI_PIN 27
#define INPUT1_PIN 14
#define INPUT2_PIN 2
#define INPUT3_PIN 15
#define OUTPUT0_PIN 26
#define OUTPUT1_PIN 25
#define OUTPUT2_PIN 33
#define OUTPUT3_PIN 32
extern WiFiClient espClient;
extern PubSubClient mqttClient;
extern std::atomic_flag mqttLock;
extern String mqttRootTopic;
extern int mqttState;
extern WiFiUDP ntpUDP;
extern NTPClient timeClient;
extern int reconnectTimeout;
extern TimeChangeRule CEST;
extern TimeChangeRule CET;
extern Timezone CE;
#define DEVICES_NUM 8
#define DEV_ALARM_MAX 5
#define DEV_ALARM_MIN  6
#define DEV_LEV_CAL    7
#define OUTPUT_BIT 0
#define MANUAL_BIT 1
#define CMD_BIT 2
#define UNACK_BIT 3
#define RUNONCE_BIT 4
#define PREVOUTPUT_BIT 5
#define SAMPLES 16
#define MQTT_CLIENTID   "eCAN-"
#define ROOT_TOPIC		"ecan/"
#define LEVEL_VAL_TOPIC	"/level/val"
#define LEVEL_MAX_TOPIC	"/level/max"
#define LEVEL_MIN_TOPIC	"/level/min"
#define A_TOPIC 	"/A"
#define B_TOPIC 	"/B"
#define C_TOPIC 	"/C"
#define D_TOPIC 	"/D"
extern char msg[];
extern float levelRaw;
extern unsigned long valveOpenSecCounters[];
extern IPAddress deviceIP;
extern bool isAP;
extern bool checkIn;
#define SD_CS_PIN 22
extern bool isSD;
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
extern int frameNo;
extern int lastFrameNo;
extern String lastMessage;
extern const char* host;
extern const char* update_path;
extern char* htmlHeader;
extern char* htmlFooter;
extern const char* serverIndex;
extern const char* www_username;
extern char www_password[];
extern char mqttServer[];
extern char mqttUser[];
extern char mqttPassword[];
extern unsigned int mqttID;
extern String mqttClientId;
extern HTTPClient http;
extern int httpCode;
extern unsigned int httpErrorCounter;
#include "libraries/interval.h"

bool loadFromSdCard(String path);
float analogRead(int pin, int samples) ;
void receivedCallback(char* topic, byte* payload, unsigned int length) ;
void mqttConnect() ;
void msOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) ;
void drawFrame1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) ;
void drawFrameA1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) ;
void drawFrameA2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) ;
void drawFrameA3(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) ;
void drawFrameA4(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) ;
void drawFrameA5(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) ;
String deviceToString(struct Device device);
void drawFrameD1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) ;
void drawFrameD2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) ;
void drawFrameD3(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) ;
void drawFrameD4(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) ;
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
void drawNextFrame(OLEDDisplay *display) ;
String int2string(int i) ;
void loopComm(void *pvParameters) ;
void loop() ;

#include "eCan.ino"


#endif
