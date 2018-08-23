//TODO: passwords to EEPROM, set time zone and year, month, day
#include "Arduino.h"
#include "libraries/WiFiManager/src/WiFiManager.h"
#include "libraries/NTPClient.h"
#include "libraries/Timezone.h"

//#define ESP8266
#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "libraries/ESP8266HTTPClient.h"
#include <ESP8266HTTPUpdateServer.h>
ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;
//const char* host = "esp8266-webupdate";
#else
#include <Update.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <HTTPClient.h>
#include <Wire.h>
#include "libraries/OLED/SSD1306.h"
#include "libraries/OLED/OLEDDisplayUi.h"
#include "images.h"
WebServer server(80);
//const char* host = "esp32-webupdate";
#endif

#include <PubSubClient.h>
#include <WiFiClient.h>
#include <DNSServer.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>

#include <FS.h>
#include "libraries/SD/src/SD.h"
#include <SPI.h>

//WiFiManager wifiManager;
/*
ESP8266
static const uint8_t D0   = 16;
static const uint8_t D1   = 5;
static const uint8_t D2   = 4;
static const uint8_t D3   = 0;
static const uint8_t D4   = 2;
static const uint8_t D5   = 14;
static const uint8_t D6   = 12;
static const uint8_t D7   = 13;
static const uint8_t D8   = 15;
static const uint8_t D9   = 3;
static const uint8_t D10  = 1;
*/

//#define ONEWIREBUS_PIN 21 //D8 //13 //D7
#ifdef ONEWIREBUS_PIN
#include "libraries/OneWire.h"
#include "libraries/DallasTemperature.h"
OneWire oneWire(ONEWIREBUS_PIN);
DallasTemperature oneWireSensors(&oneWire);
#endif

#define LED0_PIN 16 //0 //D0 //14 //D5
//#define LED1_PIN 0 //D1 //12 //D6


#define CONFIG_WIFI_PIN 27//17 //D6 //5 //D1
#define INPUT1_PIN 14 //D7 //4 //D2
#define INPUT2_PIN 2 //35 //D7 //4 //D2
#define INPUT3_PIN 15 //34 //D7 //4 //D2

#define OUTPUT0_PIN 26 //32
#define OUTPUT1_PIN 25 //33
#define OUTPUT2_PIN 33 //25
#define OUTPUT3_PIN 32 //26

//#define DHT_PIN 12 //D2 //9//6
#ifdef DHT_PIN
//#define DHTTYPE DHT11 // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
#include "libraries/DHT.h"
DHT dht(DHT_PIN, 11);
#endif

/* create an instance of PubSubClient client */
WiFiClient espClient;
PubSubClient client(espClient);
std::atomic_flag mqttLock = ATOMIC_FLAG_INIT;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
int reconnectTimeout = 0;

//Central European Time (Frankfurt, Paris)
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     //Central European Summer Time
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};       //Central European Standard Time
Timezone CE(CEST, CET);

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

//const char* mqtt_server = "broker.hivemq.com";//"iot.eclipse.org";
//const char* mqtt_server = "iot.eclipse.org";
#define ROOT_TOPIC		"ecan/"
#define LEVEL_VAL_TOPIC	"/level/val"
#define LEVEL_MAX_TOPIC	"/level/max"
#define LEVEL_MIN_TOPIC	"/level/min"
#define A_VAL_TOPIC 	"/A/val"
#define A_CMP_TOPIC     "/A/cmd" /* 1=on, 0=off */
#define B_VAL_TOPIC 	"/B/val"
#define B_CMP_TOPIC     "/B/cmd" /* 1=on, 0=off */
#define C_VAL_TOPIC 	"/C/val"
#define C_CMP_TOPIC     "/C/cmd" /* 1=on, 0=off */
#define D_VAL_TOPIC 	"/D/val"
#define D_CMP_TOPIC     "/D/cmd" /* 1=on, 0=off */
//long lastMsg = 0;
char msg[20];


bool loadFromSdCard(String path){
  String dataType = "text/plain";
  /*
  if(path.endsWith("/")) path += "index.htm";
  if(path.endsWith(".src")) path = path.substring(0, path.lastIndexOf("."));
  else if(path.endsWith(".htm")) dataType = "text/html";
  else if(path.endsWith(".css")) dataType = "text/css";
  else if(path.endsWith(".js")) dataType = "application/javascript";
  else if(path.endsWith(".png")) dataType = "image/png";
  else if(path.endsWith(".gif")) dataType = "image/gif";
  else if(path.endsWith(".jpg")) dataType = "image/jpeg";
  else if(path.endsWith(".ico")) dataType = "image/x-icon";
  else if(path.endsWith(".xml")) dataType = "text/xml";
  else if(path.endsWith(".pdf")) dataType = "application/pdf";
  else if(path.endsWith(".zip")) dataType = "application/zip";
  */
  File dataFile = SD.open(path.c_str());
  //if(dataFile.isDirectory()){
  //  path += "/index.htm";
  //  dataType = "text/html";
  //  dataFile = SD.open(path.c_str());
  //}

  if (!dataFile)
    return false;
  //if (server.hasArg("download")) dataType = "application/octet-stream";

  if (server.streamFile(dataFile, dataType) != dataFile.size()) {
    Serial.println("Sent less data than expected!");
  }

  dataFile.close();
  return true;
}

float analogRead(int pin, int samples) {
	float r = 0;
	for(int i = 0; i < samples; i++)
		r += analogRead(pin);
	r /= samples;
	return r;
}

struct Device {
	int par1;
	int par2;
	int par3;
	int par4;
	int flags;
	char name[16];
} devices[DEVICES_NUM];

float level, k, d;
unsigned long valveOpenSecCounters[DEVICES_NUM];
IPAddress deviceIP;

bool isAP;
bool checkin = false;

#define SD_CS_PIN 22
bool isSD, errorSD;

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

bool errorConn =false;

#ifndef ESP8266
// OLED pins to ESP32 GPIOs via this connection:
#define OLED_ADDRESS 0x3c
#define OLED_SDA 5 //4 // GPIO4
#define OLED_SCL 4 //15 // GPIO15
//#define OLED_RST 16 // GPIO16

SSD1306 display(OLED_ADDRESS, OLED_SDA, OLED_SCL);
//OLEDDisplayUi ui( &display );

void receivedCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("MQTT TOPIC: ");
  Serial.print(topic);

  Serial.print(" PAYLOAD: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  //Serial.println(strstr(topic, A_CMP_TOPIC)!= NULL);
  int i = -1;
  if(strstr(topic, A_CMP_TOPIC))
	  i = 0;
  if(strstr(topic, B_CMP_TOPIC))
  	  i = 1;
  if(strstr(topic, C_CMP_TOPIC))
  	  i = 2;
  if(strstr(topic, D_CMP_TOPIC))
  	  i = 3;
  if(i > -1) {
	  if((char)payload[0]=='0') {
		  bitClear(devices[i].flags, OUTPUT_BIT);
		  bitSet(devices[i].flags, MANUAL_BIT);
	  }
	  else if((char)payload[0]=='1') {
		  bitSet(devices[i].flags, OUTPUT_BIT);
		  bitSet(devices[i].flags, MANUAL_BIT);
	  }
	  //else if((char)payload[0])=='A') {
	  //	  bitClear(devices[0].flags, RUNONCE_BIT);
	  //	  bitClear(devices[0].flags, MANUAL_BIT);
	  //}
  }

  if ((char)payload[0] == '1') {
	  /* we got '1' -> on */
	  //digitalWrite(led, HIGH);
  } else {
	  /* we got '0' -> on */
	  //digitalWrite(led, LOW);
  }

}

void mqttconnect() {
  /* Loop until reconnected */
  int i = 0;
  while (!client.connected()) {
	  	yield();
		Serial.print("MQTT connecting ...");
		/* client ID */
		String clientId = "eCAN";
		/* connect now */
		if (client.connect(clientId.c_str())) {
		  Serial.println("connected");
		  /* subscribe topic with default QoS 0*/
		  client.subscribe(String(ROOT_TOPIC + String(mqttID) + "/#").c_str());
		} else {
		  Serial.print("failed, status code =");
		  Serial.println(client.state());
		  //Serial.println("try again in 5 seconds");
		  /* Wait 5 seconds before retrying */
		  delay(1000);
		}
		if(i++ >= 2)
			break;
		}
}



void msOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->setFont(ArialMT_Plain_16);
  //display->drawString(128, 0, String(millis()));
  //display->drawString(128, 0, timeClient.getFormattedTime());

  time_t t = CE.toLocal(timeClient.getEpochTime());
  byte h = (t / 3600) % 24;
  byte m = (t / 60) % 60;
  byte s = t % 60;

  char buff[10];
  sprintf(buff, "%02d:%02d:%02d", h, m, s);
  display->drawString(128, 0, buff);


  display->setTextAlignment(TEXT_ALIGN_LEFT);
  if(isAP)
	  display->drawString(0, 0, "AP");
  else {
	  if(errorConn)
		  display->drawString(0, 0, "OFFLIN");
	  else
		  display->drawString(0, 0, "ONLINE");
  }
}

#define X2 128
void drawFrame1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  // draw an xbm image.
  // Please note that everything that should be transitioned
  // needs to be drawn relative to x and y

  //display->drawXbm(x + 34, y + 14, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
	/*
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(0 + x, 10 + y, "EXT IP");
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->drawString(X2 + x, 10 + y, WiFi.localIP().toString());
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(0 + x, 20 + y, "LOC IP");
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->drawString(X2 + x, 20 + y, deviceIP.toString());
*/
  display->setFont(ArialMT_Plain_16);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  if(!isAP) { //deviceIP == IPAddress(0,0,0,0)) {
	  display->drawString(0 + x, 16 + y, WiFi.localIP().toString());
  }
  else {
	  display->drawString(0 + x, 16 + y, deviceIP.toString());
  }

  //display->setFont(ArialMT_Plain_24);
  display->setFont(ArialMT_Plain_16);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(0 + x, 32 + y, "eCAN");
}

void drawFrameA1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->setFont(ArialMT_Plain_16);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(0 + x, 16 + y, "LEVEL ALARMS");
  //display->setFont(ArialMT_Plain_24);
  display->setFont(ArialMT_Plain_16);
  /*char s[] = "              ";
  for(int i= 0; i < 2; i++) {
	  if(bitRead(devices[i+3].flags, OUTPUT_BIT)) {
		  s[i*2] = devices[i+3].name[5];
		  s[i*2 + 1] = devices[i+3].name[6];
	  }
	  else {
		  s[i*2] = ' ';
		  s[i*2 + 1] = ' ';
	  }
	  display->drawString(0 + x, 32 + y, s);
  }*/

  if(bitRead(devices[3].flags, OUTPUT_BIT))
	  display->drawString(0 + x, 32 + y, "MIN");
  else
	  display->drawString(0 + x, 32 + y, "    ");

  if(bitRead(devices[4].flags, OUTPUT_BIT))
	  display->drawString(64 + x, 32 + y, "MAX");
  else
	  display->drawString(64 + x, 32 + y, "   ");

}
void drawFrameA2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
	 if(!bitRead(devices[DEV_ALARM_MAX].flags, OUTPUT_BIT)) {
		 drawNextFrame(display);
		 return;
	 }
	 display->setFont(ArialMT_Plain_16);
	 display->setTextAlignment(TEXT_ALIGN_LEFT);
	 display->drawString(0 + x, 16 + y, "ALARM");
     display->drawString(0 + x, 32 + y, "LEVEL MAX");
}
void drawFrameA3(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
	 if(!bitRead(devices[DEV_ALARM_MIN].flags, OUTPUT_BIT)) {
		 drawNextFrame(display);
		 return;
	 }
	 display->setFont(ArialMT_Plain_16);
	 display->setTextAlignment(TEXT_ALIGN_LEFT);
	 display->drawString(0 + x, 16 + y, "ALARM");
     display->drawString(0 + x, 32 + y, "LEVEL MIN");
}
void drawFrameA4(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
	 if(!errorSD) {
		 drawNextFrame(display);
		 return;
	 }
	 display->setFont(ArialMT_Plain_16);
	 display->setTextAlignment(TEXT_ALIGN_LEFT);
	 display->drawString(0 + x, 16 + y, "ALARM");
     display->drawString(0 + x, 32 + y, "SD CARD ERR");
}
void drawFrameA5(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
	 if(!errorConn) {
		 drawNextFrame(display);
		 return;
	 }
	 display->setFont(ArialMT_Plain_16);
	 display->setTextAlignment(TEXT_ALIGN_LEFT);
	 display->drawString(0 + x, 16 + y, "ALARM");
     display->drawString(0 + x, 32 + y, "INTERNET ERR");
}

String deviceToString(struct Device device){
	//return String(bitRead(device.flags, MANUAL_BIT) ? "MAN " : "AUTO ")  + String(bitRead(device.flags, OUTPUT_BIT) ? "ON" : "OFF");
	return String(bitRead(device.flags, OUTPUT_BIT) ? "OPEN" : "CLOSE");
}
void drawFrameD1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->setFont(ArialMT_Plain_16);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(0 + x, 16 + y,"A: " + String(devices[0].name));//"VALVE A");
  //display->setFont(ArialMT_Plain_24);
  display->setFont(ArialMT_Plain_16);
  display->drawString(0 + x, 32 + y, deviceToString(devices[0]));
}
void drawFrameD2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->setFont(ArialMT_Plain_16);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(0 + x, 16 + y, "B: " + String(devices[1].name));//"VALVE B");
  //display->setFont(ArialMT_Plain_24);
  display->setFont(ArialMT_Plain_16);
  display->drawString(0 + x, 32 + y, deviceToString(devices[1]));
}
void drawFrameD3(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->setFont(ArialMT_Plain_16);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(0 + x, 16 + y, "C: " + String(devices[2].name));//"VALVE B");
  //display->setFont(ArialMT_Plain_24);
  display->setFont(ArialMT_Plain_16);
  display->drawString(0 + x, 32 + y, deviceToString(devices[2]));
}
void drawFrameD4(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->setFont(ArialMT_Plain_16);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(0 + x, 16 + y, "D: " + String(devices[3].name));//"VALVE B");
  //display->setFont(ArialMT_Plain_24);
  display->setFont(ArialMT_Plain_16);
  display->drawString(0 + x, 32 + y, deviceToString(devices[3]));
}

void drawFrameM1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  // Demonstrates the 3 included default sizes. The fonts come from SSD1306Fonts.h file
  // Besides the default fonts there will be a program to convert TrueType fonts into this format
  display->setFont(ArialMT_Plain_16);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(0 + x, 16 + y, "LEVEL");



  //display->setFont(ArialMT_Plain_24);
  display->setFont(ArialMT_Plain_16);

  level = analogRead(A0, SAMPLES) * k + d;
  display->drawString(0 + x, 32 + y, String(level));
}

// This array keeps function pointers to all frames
// frames are the single views that slide in
FrameCallback frames[] = { drawFrame1, drawFrameA2, drawFrameA3, drawFrameA4, drawFrameA5, drawFrameD1, drawFrameD2, drawFrameD3, drawFrameD4, drawFrameM1};

// how many frames are there?
int frameCount = 10;

// Overlays are statically drawn on top of a frame eg. a clock
OverlayCallback overlays[] = { msOverlay };
int overlaysCount = 1;

int lastFrameNo = 0;
String lastMessage;

void drawDisplay(OLEDDisplay *display) {
	drawDisplay(display, lastFrameNo);
}


void drawDisplay(OLEDDisplay *display, int frame) {
	lastFrameNo = frame;
	display->clear();
	//for (uint8_t i=0; i<frameCount; i++){
	(frames[frame])(display, 0, 0, 0);
	//}
	for (uint8_t i=0; i<overlaysCount; i++){
	    (overlays[i])(display, 0 );
	 }

	display->setFont(ArialMT_Plain_16);
	display->setTextAlignment(TEXT_ALIGN_LEFT);
	//display->drawString(0, 48, lastMessage);

	for(int i = 0; i < 4; i++) {
		if(bitRead(devices[i].flags, OUTPUT_BIT)) {
			char ch = 65 + i;
			display->drawString(12 * i, 48, String(ch));
		}
	}
	if(errorConn | errorSD) {
		display->setTextAlignment(TEXT_ALIGN_RIGHT);
		display->drawString(128, 48, "ALM");
		display->setTextAlignment(TEXT_ALIGN_LEFT);
	}
	/*for(int i = 0; i < 4; i++) {
		display->drawString(25 * i, 0, String(i));
		display->drawXbm(25 * i, 0, 25, 25, dropSymbol);
		if(!bitRead(devices[i].flags, OUTPUT_BIT))
			display->drawXbm(25 * i, 0, 25, 25, crossSymbol);
	}*/
	display->display();
}
void drawMessage(OLEDDisplay *display, String msg) {
	lastMessage = msg;
	//drawDisplay(display);
	//display->setFont(ArialMT_Plain_16);
	//display->setTextAlignment(TEXT_ALIGN_LEFT);
	//display->drawString(0, 48, msg);
	//display->display();
	Serial.println(msg);
}
#endif

const char* host = "eCAN-ESP8266";
const char* update_path = "/firmware";
//const char* update_username = "admin";
//const char* update_password = "admin";
//const char* ssid =  "........";
//const char* password = "........";
//ESP8266WebServer httpServer(80);


char* htmlHeader = "<html><head><title>eCAN</title><meta name=\"viewport\" content=\"width=device-width\"><style type=\"text/css\">button {height:100px;width:100px;font-family:monospace;border-radius:5px;}</style></head><body><h1><a href=/>eCAN</a></h1>";
char* htmlFooter = "<hr><a href=./save>SAVE INSTRUMENTS!</a><hr><a href=/settings>SYSTEM SETTINGS</a></body></html>";
//const char HTTP_STYLE[] PROGMEM  = "<style>.c{text-align: center;} div,input{padding:5px;font-size:1em;} input{width:95%;} body{text-align: center;font-family:verdana;} button{border:0;border-radius:0.3rem;background-color:#1fa3ec;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%;} .q{float: right;width: 64px;text-align: right;} .l{background: url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAMAAABEpIrGAAAALVBMVEX///8EBwfBwsLw8PAzNjaCg4NTVVUjJiZDRUUUFxdiZGSho6OSk5Pg4eFydHTCjaf3AAAAZElEQVQ4je2NSw7AIAhEBamKn97/uMXEGBvozkWb9C2Zx4xzWykBhFAeYp9gkLyZE0zIMno9n4g19hmdY39scwqVkOXaxph0ZCXQcqxSpgQpONa59wkRDOL93eAXvimwlbPbwwVAegLS1HGfZAAAAABJRU5ErkJggg==\") no-repeat left center;background-size: 1em;}</style>";
const char* serverIndex = "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";

const char* www_username = "eCAN";
//const char* www_password = "admin";
char www_password[20];

#ifdef THINGSSPEAK
char serverName[20];
char writeApiKey[20];
unsigned int talkbackID;
char talkbackApiKey[20];
#endif

char mqttServer[20];
char mqttUser[20];
char mqttPassword[20];
unsigned int mqttID;

HTTPClient http;  //Declare an object of class HTTPClient
int httpCode;

unsigned int errorCounter;
/*
ESP8266
static const uint8_t D0   = 16;
static const uint8_t D1   = 5;
static const uint8_t D2   = 4;
static const uint8_t D3   = 0;
static const uint8_t D4   = 2;
static const uint8_t D5   = 14;
static const uint8_t D6   = 12;
static const uint8_t D7   = 13;
static const uint8_t D8   = 15;
static const uint8_t D9   = 3;
static const uint8_t D10  = 1;
*/

//#define LCD
#ifdef LCD
#include <SPI.h>
#include "libraries/Adafruit_GFX.h"
#include "libraries/Adafruit_PCD8544.h"
// Pins
const int8_t RST_PIN = 4; //D2;
const int8_t CE_PIN = 5; //D1;
const int8_t DC_PIN = 12; //D6;
//const int8_t DIN_PIN = D7;  // Uncomment for Software SPI
//const int8_t CLK_PIN = D5;  // Uncomment for Software SPI
const int8_t BL_PIN = 16; //D0;
Adafruit_PCD8544 display = Adafruit_PCD8544(DC_PIN, CE_PIN, RST_PIN);
#endif

unsigned int r1Off = 2664494;
unsigned int r1On = 2664495;
unsigned int r2Off = 2664492;
unsigned int r2On = 2664493;
unsigned int r3Off = 2664490;
unsigned int r3On = 2664491;
unsigned int r4Off = 2664486;
unsigned int r4On = 2664487;
unsigned int rAllOff = 2664481;
unsigned int rAllOn = 2664482;


#include "libraries/RCSwitch.h"
RCSwitch rcSwitch = RCSwitch();
//#define RFRX_PIN D3





#include "libraries/interval.h"
Interval minInterval, secInterval;

unsigned long secCounter = 0;
bool secOverflow;

//#include "libraries/Time.h"
//#include "libraries/DateTime.h"






//16 //D0
//5 //D1
//4 //D2
//0 //D3
//2 //D4
//12 //D5
//14 //D6

#define EEPROM_FILANEME_ADDR 124
#define EEPROM_OFFSET 128 //64 //8

/*
strcpy(devices[0].name, "VALVE 1");
strcpy(devices[1].name, "VALVE 2");
strcpy(devices[2].name, "       ");
strcpy(devices[3].name, "LEV MIN");
strcpy(devices[4].name, "LEV MAX");
strcpy(devices[5].name, "       ");
strcpy(devices[6].name, "SETUP  ");
strcpy(devices[7].name, "       ");
 */
String getDeviceForm(int i, struct Device devices[]) {
	Device d = devices[i];
	String s = "<form action=/dev><input type=hidden name=id value=";
	s += i;
	s += "><h2>";
	s += char( i+ 65);
	s += ": ";
	s += String(d.name);
	if(i<DEV_ALARM_MAX) {
		//if(bitRead(devices[i].flags, MANUAL_BIT))
		//	s += " MANUAL";
		//else
		//	s += " AUTO";
		if(bitRead(devices[i].flags, OUTPUT_BIT))
			s += " OPEN";
		else
			s += " CLOSE";

	}
	s += "</h2>";

	if(i<DEV_ALARM_MAX) {
		s += "<button type=submit name=cmd value=off>CLOSE</button>&nbsp;&nbsp;&nbsp;<button type=submit name=cmd value=on>OPEN</button>&nbsp;&nbsp;&nbsp;<button type=submit name=cmd value=auto>AUTO</button>";
	}
	s += "<hr><h3>SETTINGS:</h3>";
	s += "<hr>NAME<br>";
	s += "<input name=name value=\"";
	s += d.name;
	s += "\">";
	//s += d.name;
	if(i < DEV_ALARM_MAX) {
		s += "<hr>OPEN AT [HOUR]<br><input name=par1 value=";
		s += d.par1;
		s += "><hr>OPEN AT [MINUTE]<br><input name=par2 value=";
		s += d.par2;
		s += "><hr>CLOSE AFTER [MINUTE]<br><input name=par3 value=";
		s += d.par3;
		s += "><hr>CLOSE AFTER [SECOND]<br><input name=par4 value=";
		s += d.par4;
		s += ">";
	}
	if(i==DEV_ALARM_MAX) {
			s += "<hr>VALVES OPEN LEVEL [mm]<br><input name=par1 value=";
			s += d.par1;
			//s += "><hr>VALVES OPEN LEVEL [mm]<br><input name=par2 value=";
			//s += d.par2;
			s += "><hr><br>ALARM MAX LEVEL [mm]<br><input name=par3 value=";
			s += d.par3;
			s += "><hr><br>ALARM HYST [mm]<br><input name=par4 value=";
			s += d.par4;
			s += ">";
	}
	if(i==DEV_ALARM_MIN) {
		s += "<hr>VALVES CLOSE LEVEL [mm]<br><input name=par1 value=";
		s += d.par1;
		//s += "><hr>VALVE CLOSE LEVEL [mm]<br><input name=par2 value=";
		//s += d.par2;
		s += "><hr><br>ALARM MIN LEVEL [mm]<br><input name=par3 value=";
		s += d.par3;
		s += "><hr><br>ALARM HYST [mm]<br><input name=par4 value=";
		s += d.par4;
		s += ">";
	}

	if(i==DEV_LEV_CAL) {
		s += "<hr>X0 [-]<br><input name=par1 value=";
		s += d.par1;
		s += "><hr>X100 [-]<br><input name=par2 value=";
		s += d.par2;
		s += "><hr>Y0 [mm]<br><input name=par3 value=";
		s += d.par3;
		s += "><hr>Y100 [mm]<br><input name=par4 value=";
		s += d.par4;
		s += ">";
	}
	if(i==7) {
	}
	s += "<hr><button type=submit name=cmd value=set>SET</button>";
	s += "</form>";
	return s;
}

void handleRoot(){
   if(!server.authenticate(www_username, www_password))
      return server.requestAuthentication();

  Serial.println("Enter handleRoot");
  String message;
  server.send(200, "text/plain", message);
}

bool led1;

void saveApi() {
	int offset = 8;
	EEPROM.put(offset, www_password);
	offset += sizeof(www_password);
#ifdef THINGSSPEAK
	EEPROM.put(offset, serverName);
	offset += sizeof(serverName);
	EEPROM.put(offset, writeApiKey);
	offset += sizeof(writeApiKey);
	EEPROM.put(offset, talkbackApiKey);
	offset += sizeof(talkbackApiKey);
	EEPROM.put(offset, talkbackID);
	offset += sizeof(talkbackID);
#endif
	EEPROM.put(offset, mqttServer);
	offset += sizeof(mqttServer);
	EEPROM.put(offset, mqttUser);
	offset += sizeof(mqttUser);
	EEPROM.put(offset, mqttPassword);
	offset += sizeof(mqttPassword);
	EEPROM.put(offset, mqttID);
	offset += sizeof(mqttID);

	EEPROM.put(0, 0);
	EEPROM.commit();

	client.setServer(mqttServer, 1883);
}

void saveInstruments() {
	for(int d=0; d< DEVICES_NUM; d++) {
		EEPROM.put(EEPROM_OFFSET + sizeof(Device) * d, devices[d]);
	}
	EEPROM.put(0, 0);
	EEPROM.commit();
}

void startWiFiAP() {

	  isAP = true;
	  WiFi.softAP("eCAN", "eCANeCANeCAN");
	  //IPAddress deviceIP = WiFi.softAPIP();
	  Serial.println("Starting AP ...");
	  deviceIP = WiFi.softAPIP();
	  Serial.print("AP IP address: ");
	  Serial.println(deviceIP);
}


void setup() {
  Serial.begin(115200);
  Serial.print("\n\n");
  Serial.println("eCAN");

#ifndef ESP8266
  analogReadResolution(9);
  display.init();
  display.setFont(ArialMT_Plain_24);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 0, "eCAN");
  display.display();
#endif


  isSD = true;
  if(!SD.begin(SD_CS_PIN)) {
	  isSD = false;
      Serial.println("Card Mount Failed");
  }
  uint8_t cardType = SD.cardType();
  if(cardType == CARD_NONE){
	  isSD = false;
	  Serial.println("No SD card attached");
  }

#ifdef LCD
  display.begin();
  display.setContrast(60);  // Adjust for your display
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(0,0);
  display.println("eCAN");
  display.display();
#endif

#ifdef ONEWIRE_PIN
  oneWireSensors.begin();
#endif
#ifdef DHT_PIN
  dht.begin();
#endif

#ifdef RFRX_PIN
  rcSwitch.enableReceive(RFRX_PIN);
#endif

//#define RFTX_PIN 22 //D5
#ifdef RFTX_PIN
  rcSwitch.enableTransmit(RFTX_PIN);
#endif

  EEPROM.begin(512);
  if(!EEPROM.read(0)) {
  //if(false) {
	  int offset = 8;

	  EEPROM.get(offset, www_password);
	  offset += sizeof(www_password);
#ifdef THINGSSPEAK
	  EEPROM.get(offset, serverName);
	  offset += sizeof(serverName);
      EEPROM.get(offset, writeApiKey);
      offset += sizeof(writeApiKey);
      EEPROM.get(offset, talkbackApiKey);
      offset += sizeof(talkbackApiKey);
	  EEPROM.get(offset, talkbackID);
	  offset += sizeof(talkbackID);
#endif
	  EEPROM.get(offset, mqttServer);
	  offset += sizeof(mqttServer);
	  EEPROM.get(offset, mqttUser);
	  offset += sizeof(mqttUser);
	  EEPROM.get(offset, mqttPassword);
	  offset += sizeof(mqttPassword);
	  EEPROM.get(offset, mqttID);
	  offset += sizeof(mqttID);

	  client.setServer(mqttServer, 1883);

	  for(int d=0; d< DEVICES_NUM; d++) {
		  EEPROM.get(EEPROM_OFFSET + sizeof(Device) * d, devices[d]);
		  bitClear(devices[d].flags, OUTPUT_BIT);
	  }
  }
  else {
	  strcpy(www_password, "eCAN") ;
#ifdef THINGSSPEAK
	  strcpy(serverName, "api.thingspeak.com") ;
	  writeApiKey[0] = '/0';
	  talkbackID = 0;
	  talkbackApiKey[0] = '/0';
#endif
	  strcpy(mqttServer, "broker.hivemq.com") ;
	  //mqttUser[0] = '/0';
	  //mqttPassword[0] = '/0';
	  strcpy(mqttUser, "eCAN") ;
	  strcpy(mqttPassword, "eCAN") ;
	  mqttID = 0;

	  saveApi();
	  saveInstruments();


	  strcpy(devices[0].name, "VALVE A");
	  strcpy(devices[1].name, "VALVE B");
	  strcpy(devices[2].name, "VALVE C");
	  strcpy(devices[3].name, "VALVE D");
	  strcpy(devices[4].name, "       ");
  }
  strcpy(devices[DEV_ALARM_MAX].name, "LEVEL MAX");
  strcpy(devices[DEV_ALARM_MIN].name, "LEVEL MIN");
  strcpy(devices[DEV_LEV_CAL].name,   "LEVEL");



#ifdef CONFIG_WIFIAP_PIN
  pinMode(CONFIG_WIFIAP_PIN, INPUT_PULLUP);
#endif

#ifdef INPUT1_PIN

  pinMode(CONFIG_WIFI_PIN, INPUT_PULLUP);
  pinMode(INPUT1_PIN, INPUT_PULLUP);
  pinMode(INPUT2_PIN, INPUT_PULLUP);
  pinMode(INPUT3_PIN, INPUT_PULLUP);
#endif

#ifdef RFTX_PIN
  pinMode(RFTX_PIN, OUTPUT);
#endif

#ifdef OUTPUT0_PIN
  pinMode(OUTPUT0_PIN, OUTPUT);
  pinMode(OUTPUT1_PIN, OUTPUT);
  pinMode(OUTPUT2_PIN, OUTPUT);
  pinMode(OUTPUT3_PIN, OUTPUT);
  digitalWrite(OUTPUT0_PIN, HIGH);
  digitalWrite(OUTPUT1_PIN, HIGH);
  digitalWrite(OUTPUT2_PIN, HIGH);
  digitalWrite(OUTPUT3_PIN, HIGH);
#endif

#ifdef LED0_PIN
  pinMode(LED0_PIN, OUTPUT);
  digitalWrite(LED0_PIN, LOW);
#endif
#ifdef LED1_PIN
  pinMode(LED1_PIN, OUTPUT);
  digitalWrite(LED1_PIN, LOW);
#endif


  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  //reset saved settings
  //wifiManager.resetSettings();
  //set custom ip for portal
  //wifiManager.setAPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
  //fetches ssid and pass from eeprom and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration

  //if(digitalRead(CONFIG_WIFIAP_PIN) == HIGH) {

 if ( digitalRead(INPUT1_PIN) == LOW ) {
  strcpy(www_password, "eCAN") ;
 }

  //if(0) {
  if ( digitalRead(CONFIG_WIFI_PIN) == LOW ) {
  //if ( digitalRead(CONFIG_WIFI_PIN) == HIGH ) {
#ifdef LED0_PIN
	  digitalWrite(LED0_PIN, HIGH);
#endif



	  Serial.println("Starting AP for reconfiguration ...");
#ifndef ESP8266
	  drawMessage(&display, "CONFIG WIFI!");
	  drawDisplay(&display,  0);
#endif
	  wifiManager.resetSettings();
	  wifiManager.startConfigPortal("eCAN");
  }
  else {

	  isAP = true;
	  Serial.println("Starting AP or connecting to Wi-Fi ...");
	  for(int i = 0; i < 3; i++) {
#ifndef ESP8266
		  drawMessage(&display, "CONN WIFI ... " + String(i));
#endif
		  if(wifiManager.autoConnect()) {
			  isAP = false;
			  Serial.print("IP address: ");
			  Serial.println(WiFi.localIP());
#ifndef ESP8266
			  drawMessage(&display, "CONN WIFI");
#endif
			  break;
		  }
		  Serial.println(wifiManager.getSSID());
		  Serial.println(wifiManager.getPassword());
	  }

	  if(isAP) {
			startWiFiAP();
	  }
#ifdef LED1_PIN
	  digitalWrite(LED1_PIN, HIGH);
#endif
  }


  Serial.println("Ready");
  WiFi.printDiag(Serial);
#ifndef ESP8266
  drawMessage(&display, "WIFI READY");
#endif


  /*
  //-----------
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if(WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Connect Failed! Rebooting...");
    delay(1000);
    ESP.restart();
  }*/
  //ArduinoOTA.begin();

  /* configure the MQTT server with IPaddress and port */
  //client.setServer(mqtt_server, 1883);
  /* this receivedCallback function will be invoked	when client received subscribed topic */
  client.setCallback(receivedCallback);

  //timeClient.setUpdateInterval(60000 * 10);
  mqttLock.clear();

  server.on("/", [](){
    Serial.println("/");
    if(!server.authenticate(www_username, www_password))
      return server.requestAuthentication();

    //String message = "<HTML><BODY><H1>esp8266</H1><A HREF=set?state=on>ON</A><BR><A HREF=set?state=off>OFF</A></BODY></HTML>";
    //String message = "<html><head><meta name=\"viewport\" content=\"width=device-width\"></head><body><h1>esp8266</h1>";
    String message = htmlHeader;

    //message += "Time: " + timeClient.getFormattedTime();
    time_t t = CE.toLocal(timeClient.getEpochTime());

    byte h = (t / 3600) % 24;
    byte m = (t / 60) % 60;
    byte s = t % 60;
    message += "<h2>TIME: ";
    if(h<10)
    	message += '0';
    message += h;
    message += ":";
    if(m<10)
        	message += '0';
    message += m;
    message += ":";
    if(s<10)
        	message += '0';
    message += s;
    message += "</h2>";

    message += "<h2>LEVEL: ";
    message += level;
    message += " mm</h2>";

    if(bitRead(devices[DEV_ALARM_MAX].flags, OUTPUT_BIT))
    	message += "<h2>ALARM: LEVEL MAX</h2>";
    if(bitRead(devices[DEV_ALARM_MIN].flags, OUTPUT_BIT))
    	message += "<h2>ALARM: LEVEL MIN</h2>";
    if(errorConn)
    	message += "<h2>ALARM: INTERNET CONNECTION ERROR</h2>";
    if(errorSD)
    	message += "<h2>ALARM: SD CARD</h2>";

    for(int i = 0; i < DEVICES_NUM; i++) {
    	if(i == 4) // || i == 5 || i == 7)
    		continue;

    	if(i == 0) {
    		message += "<hr><h3>VALVES</h3>";
    	}
    	if(i == DEV_ALARM_MAX) {
			message += "<hr><h3>LIMITS LEVEL</h3>";
		}
    	if(i == DEV_LEV_CAL) {
			message += "<hr><h3>CALIBRATION</h3>";
		}

    	message += "<hr><a href=./dev?id=";
    	//message += "<a href=./dev?id=";
    	message += i;
    	message += ">";
    	if(i < DEV_ALARM_MAX) {
			message += char(i + 65);
			message += ": ";
    	}
    	message += devices[i].name;
    	//message += "</A> ";
    	if(i < DEV_ALARM_MAX) {
			//if(bitRead(devices[i].flags, MANUAL_BIT))
			//	message += " MANUAL";
			//else
			//	message += " AUTO";
			if(bitRead(devices[i].flags, OUTPUT_BIT))
				message += " OPEN";
			else
				message += " CLOSE";
		}
    	if(i == 3 || i == 4) {
			//if(bitRead(devices[i].flags, OUTPUT_BIT))
			//			message += " ALARM ON";
			//		else
			//			message += " ALARM OFF";
    	}
    	message += "</a>";
    }
    //message += "</body></html>";

    message += "<hr><h3>SYSTEM</h3>";
    message += "<hr><a href=/logs>LOGS</a>";
    //message += "<hr><a href=./save>SAVE INSTRUMENTS!</a>";
    message += htmlFooter;
    server.send(200, "text/html", message);
  });

  /*
  server.on("/dev", [](){
	  Serial.println("/dev");
	  byte id=server.arg("id").toInt();
	  String message = "<HTML><BODY><H1>esp8266</H1>";
	  message += getDeviceForm(id, devices);
	  message += "</BODY></HTML>";
	  server.send(200, "text/html", message);
   });
   */

  server.on("/log", [](){
  	  Serial.println("/log");
  	  Serial.println(server.args());
        if(!server.authenticate(www_username, www_password))
      	  return server.requestAuthentication();
      if(server.args() > 0)
    	  loadFromSdCard(server.arg(0));
  });

  server.on("/logs", [](){
	  Serial.println("/logs");
      if(!server.authenticate(www_username, www_password))
    	  return server.requestAuthentication();

      String message = htmlHeader;


      //Serial.printf("Listing directory: %s\n", dirname);

	  File root = SD.open("/");
	  if(!root){
		  Serial.println("Failed to open directory");
		  return;
	  }
	  if(!root.isDirectory()){
		  Serial.println("Not a directory");
		  return;
	  }

	  message += "<table>";
	  File file = root.openNextFile();
	  while(file){
		  if(file.isDirectory()){
			  Serial.print("DIR: ");
			  Serial.println(file.name());
			  //if(levels){
			  //  listDir(fs, file.name(), levels -1);
		  }
		  else {
			  Serial.print("FILE: ");
			  Serial.print(file.name());
			  Serial.print("SIZE: ");
			  Serial.println(file.size());

			  message += "<tr><td><a href=/log?name=";
			  message += file.name();
			  message += ">";
			  message += file.name();
			  message += "</a></td><td>";
			  message += String(file.size());
			  message += "</td></tr>";

		  }
		  file = root.openNextFile();
	  }
	  message += "</table>";
	  Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
	  Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));



	  message += htmlFooter;
	  server.send(200, "text/html", message);

  });

  server.on("/save", [](){
      Serial.println("/save");
      if(!server.authenticate(www_username, www_password))
        return server.requestAuthentication();

      saveInstruments();

      char value;
      for(int i=0; i < 512; i++) {
    	  //EEPROM.write(addr, val);
    	  if(i % 32 == 0)
    		  Serial.println();
    	  value = EEPROM.read(i);
    	  Serial.print(value, HEX);
    	  Serial.print(' ');
      }

      String message = htmlHeader;
      message += "OK";
      message += htmlFooter;
      server.send(200, "text/html", message);

  });

  server.on("/settings", [](){
        Serial.println("/settings");
        if(!server.authenticate(www_username, www_password))
          return server.requestAuthentication();

        time_t t = CE.toLocal(timeClient.getEpochTime());
		byte h = (t / 3600) % 24;
		byte m = (t / 60) % 60;
		byte s = t % 60;

        String message = htmlHeader;
        message += "<hr>";
        message += "<form action=/savesettings>";
        message += "HOURS<br><input name=hours value=";
        //message += timeClient.getHours();
        message += h;
        message += "><br>";
        message += "<br>MINUTES<br><input name=minutes value=";
		//message += timeClient.getMinutes();
        message += m;
		message += "><br>";
        message += "<br>SECONDS<br><input name=seconds value=";
        //message += timeClient.getSeconds();
        message += s;

        message += "><br>";
		message += "<br>YEAR<br><input name=year value=";
		//message += timeClient.getMinutes();
		//message += time_t;
        message += "><br>";
		message += "<br>MONTH<br><input name=month value=";
		//message += timeClient.getMinutes();
		//message += m;
        message += "><br>";
		message += "<br>DAY<br><input name=day value=";
		//message += timeClient.getMinutes();
		//message += m;


        message += "><br><br>";
        message += "<button type=submit name=cmd value=settime>SET TIME</button>";
        message += "</form>";
        message += "<hr>";

        message += "<form action=/savesettings>";
        message += "ADMIN PASSWORD<br><input name=www_password value=";
      	message += www_password;
      	message += "><br>";
#ifdef THINGSSPEAK
        message += "<br>SERVER NAME<br><input name=servername value=";
		message += serverName;
		message += "><br>";
	    message += "<br>WRITE API KEY<br><input name=writeapikey value=";
	    message += writeApiKey;
	    message += "><br>";
	    message += "<br>TALKBACK ID<br><input name=talkbackid value=";
		message += talkbackID;
		message += "><br>";
	    message += "<br>TALKBACK API KEY<br><input name=talkbackapikey value=";
	    message += talkbackApiKey;
	    message += "><br>";
#endif
	    message += "<br>MQTT SERVER<br><input name=mqttserver value=";
		message += mqttServer;
		message += "><br>";
		message += "<br>MQTT USER<br><input name=mqttuser value=";
		message += mqttUser;
		message += "><br>";
		message += "<br>MQTT PASSWORD<br><input name=mqttpassword value=";
		message += mqttPassword;
		message += "><br>";
	    message += "<br>MQTT ID<br><input name=mqttid value=";
	    message += mqttID;
	    message += "><br><br>";
	    message += "<button type=submit name=cmd value=setapi>SET API!</button>";
	    message += "</form>";

        message += htmlFooter;
        server.send(200, "text/html", message);

    });

  server.on("/savesettings", [](){
          Serial.println("/savesettings");

          if(!server.authenticate(www_username, www_password))
            return server.requestAuthentication();

          String message = htmlHeader;
          if(server.arg("cmd").equals("settime")) {
        	  int offset = CE.toUTC(0) - CE.toLocal(0);
        	  //Serial.println(offset);
        	  unsigned long h=(unsigned long)server.arg("hours").toInt();// + offset / 3600;
        	  unsigned long m=(unsigned long)server.arg("minutes").toInt();// + offset / 60;
        	  unsigned long s=(unsigned long)server.arg("seconds").toInt();// + offset;

        	  Serial.println("Old time:");
        	  Serial.println(timeClient.getEpochTime());
        	  Serial.println(timeClient.getFormattedTime());
        	  timeClient.setEpochTime((h * 3600 + m * 60 + s) - timeClient.getEpochTime());// + offset);
        	  Serial.println("New time:");
        	  Serial.println(timeClient.getEpochTime());
        	  Serial.println(timeClient.getFormattedTime());
			  Serial.println(h);
			  Serial.println(m);
			  Serial.println(s);
			  Serial.println(h * 3600 + m * 60 + s);

			  message += "TIME SET";
  	  	  }

          if(server.arg("cmd").equals("setapi")) {
        	  //TODO: disable for demo
        	  strcpy(www_password, server.arg("www_password").c_str());

#ifdef THINGSSPEAKS
        	  strcpy(serverName, server.arg("servername").c_str());
        	  strcpy(writeApiKey, server.arg("writeapikey").c_str());
        	  talkbackID = server.arg("talkbackid").toInt();
        	  strcpy(talkbackApiKey, (char*)server.arg("talkbackapikey").c_str());
#endif
        	  strcpy(mqttServer, (char*)server.arg("mqttserver").c_str());
        	  strcpy(mqttUser, (char*)server.arg("mqttuser").c_str());
        	  strcpy(mqttPassword, (char*)server.arg("mqttpassword").c_str());
        	  mqttID = server.arg("mqttid").toInt();

        	  message += "API SET";

        	  saveApi();
          }

          message += htmlFooter;
          server.send(200, "text/html", message);

      });



  server.on("/dev", [](){
    Serial.println("/dev");
    if(!server.authenticate(www_username, www_password))
      return server.requestAuthentication();

    String cmd=server.arg("cmd");
    Serial.println(cmd);

	byte id=server.arg("id").toInt();
    if(cmd.equals("set")) {
    	byte par1=server.arg("par1").toInt();
    	byte par2=server.arg("par2").toInt();
    	byte par3=server.arg("par3").toInt();
    	byte par4=server.arg("par4").toInt();
    	String name=server.arg("name");

    	if(id >=0 && id < DEVICES_NUM) {
    		devices[id].par1 = par1;
    		devices[id].par2 = par2;
    		devices[id].par3 = par3;
    		devices[id].par4 = par4;
    		strncpy(devices[id].name, name.c_str(), 16);
    		//name.toCharArray(devices[id].name, 8);
    	}
    }
    if(cmd.equals("auto")) {
    	bitClear(devices[id].flags, MANUAL_BIT);
    	bitClear(devices[id].flags, RUNONCE_BIT);
    }
    if(cmd.equals("off")) {
    	bitSet(devices[id].flags, MANUAL_BIT);
    	bitClear(devices[id].flags, CMD_BIT);
    	bitClear(devices[id].flags, OUTPUT_BIT);
    }
    if(cmd.equals("on")) {
    	bitSet(devices[id].flags, MANUAL_BIT);
    	bitSet(devices[id].flags, CMD_BIT);
    	bitSet(devices[id].flags, OUTPUT_BIT);
    }

	//String message = "<html><head><meta name=\"viewport\" content=\"width=device-width\"><style type=\"text/css\">button {height:100px;width:100px;font-family:monospace;border-radius:5px;}</style></head><body><h1><a href=/>esp8266</a></h1>";
    String message = htmlHeader;
	message += getDeviceForm(id, devices);
	//message += "</body></html>";
	message += htmlFooter;

    server.send(200, "text/html", message);
  });
#ifndef ESP8266
  //const char* serverIndex = "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";
  server.on("/update", HTTP_GET, [](){
        if(!server.authenticate(www_username, www_password))
        	return server.requestAuthentication();
        server.sendHeader("Connection", "close");

        String message = htmlHeader;
		message += serverIndex;
		message += htmlFooter;
		server.send(200, "text/html", message);

        //server.send(200, "text/html", "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>");
      });

  server.on("/update", HTTP_POST, [](){

        if(!server.authenticate(www_username, www_password))
            return server.requestAuthentication();
        server.sendHeader("Connection", "close");\
        server.send(200, "text/plain", (Update.hasError())?"FAIL":"OK");
        ESP.restart();
      },[](){
        HTTPUpload& upload = server.upload();
        if(upload.status == UPLOAD_FILE_START){
          Serial.setDebugOutput(true);
#ifdef ESP8266
          //WiFiUDP::stopAll();
#endif
          Serial.printf("Update: %s\n", upload.filename.c_str());
#ifdef ESP8266
          uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
#else
          uint32_t maxSketchSpace = 0x140000;//(ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
#endif
          if(!Update.begin(maxSketchSpace)){//start with max available size
            Update.printError(Serial);
          }
        } else if(upload.status == UPLOAD_FILE_WRITE){
          if(Update.write(upload.buf, upload.currentSize) != upload.currentSize){
            Update.printError(Serial);
          }
        } else if(upload.status == UPLOAD_FILE_END){
          if(Update.end(true)){ //true to set the size to the current progress
            Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
          } else {
            Update.printError(Serial);
          }
          Serial.setDebugOutput(false);
        }
        yield();
      });

#endif

  server.begin();

  //timeClient.update();

  //MDNS.begin(host);
  MDNS.begin(host);
#ifdef ESP8266
  httpUpdater.setup(&server, update_path, www_username, www_password);
#endif
  //httpServer.begin();
  MDNS.addService("http", "tcp", 80);


  // Port defaults to 8266
  //ArduinoOTA.setPort(8266);
  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(host);
  // No authentication by default
  //ArduinoOTA.setPassword((const char *)"xxxxx");
  ArduinoOTA.onStart([]() {
	Serial.println("OTA Start");
  });
  ArduinoOTA.onEnd([]() {
	Serial.println("OTA End");
    Serial.println("Rebooting...");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r\n", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();

  if ( digitalRead(CONFIG_WIFI_PIN) == LOW ) {
	  //emergency restore OTA
	  while(true) {
		  ArduinoOTA.handle();
		  server.handleClient();
	  }
  }


#ifndef ESP8266
  xTaskCreatePinnedToCore(loop1, "loop1", 4096, NULL, 1, NULL, ARDUINO_RUNNING_CORE);
#else

#endif
}



int frameWait = 0;
int frameNo = 0;

void drawNextFrame(OLEDDisplay *display) {
	frameNo++;
	drawDisplay(display, frameNo);
}

String int2string(int i) {
	if(i < 10)
		return "0" + String(i);
	return String(i);
}

void loop1(void *pvParameters) {

#ifndef ESP8266
	 while (1) {
#else
	if(1) {
#endif

#ifndef ESP8266
		 drawMessage(&display, "CONNECTING ...");
		 drawMessage(&display, "CONN NTP ...");
#endif
		 timeClient.update();
#ifndef ESP8266
		 drawMessage(&display, "NTP DONE");
#endif

		 errorConn = true;
		 //drawMessage(&display, String(millis()));





		 if(!mqttLock.test_and_set() && mqttServer[0] != 0 ) {




#ifndef ESP8266
		    drawMessage(&display, "MQTT CONNECTING...");
#endif




			 if (!client.connected()) {
				mqttconnect();
			  }
			 /* this function will listen for incomming subscribed topic-process-invoke receivedCallback */
			 //client.loop();

			 if (client.connected()) {
				 snprintf (msg, 20, "%lf", level);
				 //client.publish(LEVEL_VAL_TOPIC, msg);

				 client.publish(String(ROOT_TOPIC + String(mqttID) + LEVEL_VAL_TOPIC).c_str(), msg);
				 Serial.println(String(ROOT_TOPIC + String(mqttID) + LEVEL_VAL_TOPIC).c_str());
				 Serial.println(msg);

				 snprintf (msg, 20, "%d", bitRead(devices[DEV_ALARM_MAX].flags, OUTPUT_BIT));
				 //client.publish(LEVEL_MAX_TOPIC, msg);
				 client.publish(String(ROOT_TOPIC + String(mqttID) + LEVEL_MAX_TOPIC).c_str(), msg);

				 snprintf (msg, 20, "%d", bitRead(devices[DEV_ALARM_MIN].flags, OUTPUT_BIT));
				 //client.publish(LEVEL_MIN_TOPIC, msg);
				 client.publish(String(ROOT_TOPIC + String(mqttID) + LEVEL_MIN_TOPIC).c_str(), msg);

				 snprintf (msg, 20, "%d", bitRead(devices[0].flags, OUTPUT_BIT));
				 //client.publish(A_VAL_TOPIC, msg);
				 client.publish(String(ROOT_TOPIC + String(mqttID) + A_VAL_TOPIC).c_str(), msg);

				 snprintf (msg, 20, "%d", bitRead(devices[1].flags, OUTPUT_BIT));
				 //client.publish(B_VAL_TOPIC, msg);
				 client.publish(String(ROOT_TOPIC + String(mqttID) + B_VAL_TOPIC).c_str(), msg);

				 snprintf (msg, 20, "%d", bitRead(devices[2].flags, OUTPUT_BIT));
				 //client.publish(C_VAL_TOPIC, msg);
				 client.publish(String(ROOT_TOPIC + String(mqttID) + C_VAL_TOPIC).c_str(), msg);

				 snprintf (msg, 20, "%d", bitRead(devices[3].flags, OUTPUT_BIT));
				 //client.publish(D_VAL_TOPIC, msg);
				 client.publish(String(ROOT_TOPIC + String(mqttID) + D_VAL_TOPIC).c_str(), msg);

				 errorConn = false;
#ifndef ESP8266
				 drawMessage(&display, "MQTT DONE");
#endif
		 	 }
			 else {
#ifndef ESP8266
				 drawMessage(&display, "MQTT ERROR");
#endif
			 }
			 mqttLock.clear();
		 }

		 if(isSD) {
#ifndef ESP8266
  			  drawMessage(&display, "SD LOGGING...");
#endif

  			 int fileIndex = 0;
  			 String path = "/" + String(fileIndex) + ".csv";
			 //char* path = "/log1.csv";
			 //String message = timeClient.getFormattedTime() + String(bitRead(devices[2].flags, OUTPUT_BIT) | bitRead(devices[3].flags, OUTPUT_BIT)) +  ';' + String(level) + ';' + String(bitRead(devices[0].flags, OUTPUT_BIT)) + ';' + String(bitRead(devices[1].flags, OUTPUT_BIT)) + '\n';;
  			 time_t t = CE.toLocal(timeClient.getEpochTime());

  			 String message = String(year(t)) + "-" + int2string(month(t)) + "-" + int2string(day(t)) + " " + int2string(hour(t)) + ":" + int2string(minute(t)) + ":" + int2string(second(t)) + ";"
  					 	 + String(bitRead(devices[DEV_ALARM_MAX].flags, OUTPUT_BIT) | bitRead(devices[DEV_ALARM_MIN].flags, OUTPUT_BIT)) + ";"
						 + String(level) + ";" + String(bitRead(devices[0].flags, OUTPUT_BIT)) + ";" + String(bitRead(devices[1].flags, OUTPUT_BIT)) + ";" + String(bitRead(devices[2].flags, OUTPUT_BIT)) + ";" + String(bitRead(devices[3].flags, OUTPUT_BIT)) + '\n';
  			 Serial.print(message);
			 errorSD = false;
			 if(!SD.exists(path)) {
				 File file = SD.open(path, FILE_APPEND);
				 if(file) {
					 file.print("DATE TIME;ALARMS;LEVEL[mm];A: " + String(devices[0].name) + ";B: " + String(devices[1].name) +";C: " + String(devices[2].name) + ";D: " + String(devices[3].name) + "\n");
					 file.close();
				 }
				 else {
					 Serial.println("Failed to create file");
				 }
			 }
			 File file = SD.open(path, FILE_APPEND);
			 if(!file){
				 Serial.println("Failed to open file for appending");
			     //return;
			 }
			 if(!file) {
				 errorSD = true;
			 }

			 if(file) {
			     if(file.print(message)){
			    	 //errorSD = false;
			         //Serial.println("Message appended");
			     }
			     else {
			    	 errorSD = true;
			         Serial.println("Append failed");
			     }
			     file.close();
			 }
#ifndef ESP8266
			 if(errorSD)
				 drawMessage(&display, "SD ERROR");
			 else
				 drawMessage(&display, "SD DONE");
#endif
		 }



		 if(!isAP) {



			  if(!checkin) {
#ifndef ESP8266
				  drawMessage(&display, "CONN GM ...");
#endif
				  http.begin("http://growmat.cz/growmatweb/dev/");
				  httpCode = http.GET();
				  if(httpCode > 0)
					  checkin = true;
#ifndef ESP8266
				  drawMessage(&display, "GM DONE");
#endif
			  }

#ifdef THINGSSPEAK
			  if(serverName[0] != 0 && !isAP) {

				  //HTTPClient http;  //Declare an object of class HTTPClient

				  //int httpCode;
				  do {
					  //http.begin("http://api.thingspeak.com/talkbacks/18221/commands/execute?api_key=ED57BAC7V0GLPWL2");
					  //String getTalkback = "http://api.thingspeak.com/talkbacks/" + String(talkbackID) + "/commands/execute?api_key=" + talkbackApiKey;
#ifndef ESP8266
					  drawMessage(&display, "CONN TB ...");
#endif
					  String getTalkback = "http://" + String(serverName) + "/talkbacks/" + String(talkbackID) + "/commands/execute?api_key=" + talkbackApiKey;
					  Serial.println(getTalkback);

					  //TODO:
					  http.begin(getTalkback);
					  httpCode = http.GET();                                                                  //Send the request
					  Serial.println(httpCode);
					  Serial.println(http.errorToString(httpCode));
#ifndef ESP8266
					  drawMessage(&display, "TB DONE");
#endif
					  if(httpCode != 200) {
						  errorCounter++;
						  errorConn = true;
					  }
					  else {
						  errorConn = false;
					  }


					  if (httpCode > 0) { //Check the returning code
						  String payload = http.getString();   //Get the request response payload
						  Serial.println(payload);                     //Pri
						  if(payload == "")
							  break;
						  if(payload == "error_auth_required")
							  break;
						  if(payload.charAt(0)=='A') {
							  if(payload.charAt(1)=='0') {
								  bitClear(devices[0].flags, OUTPUT_BIT);
								  bitSet(devices[0].flags, MANUAL_BIT);
							  }
							  else if(payload.charAt(1)=='1') {
								  bitSet(devices[0].flags, OUTPUT_BIT);
								  bitSet(devices[0].flags, MANUAL_BIT);
							  }
							  else if(payload.charAt(1)=='A') {
								  bitClear(devices[0].flags, RUNONCE_BIT);
								  bitClear(devices[0].flags, MANUAL_BIT);
							  }
						  }
						  if(payload.charAt(0)=='B') {
							  if(payload.charAt(1)=='0') {
								  bitClear(devices[1].flags, OUTPUT_BIT);
								  bitSet(devices[1].flags, MANUAL_BIT);
							  }
							  else if(payload.charAt(1)=='1') {
								  bitSet(devices[1].flags, OUTPUT_BIT);
								  bitSet(devices[1].flags, MANUAL_BIT);
							  }
							  else if(payload.charAt(1)=='A') {
								  bitClear(devices[1].flags, RUNONCE_BIT);
								  bitClear(devices[1].flags, MANUAL_BIT);
							  }
						  }
					  }
					  else
						  break;
				  } while (true);


				  String alarm = String(bitRead(devices[3].flags, OUTPUT_BIT) * 1 + bitRead(devices[4].flags, OUTPUT_BIT) * 10);
				  String v1 = String(bitRead(devices[0].flags, OUTPUT_BIT) + bitRead(devices[0].flags, MANUAL_BIT) * 10);
				  String v2 = String(bitRead(devices[1].flags, OUTPUT_BIT) + bitRead(devices[1].flags, MANUAL_BIT) * 10);
				  //String get = GET + "SGRXDOXDL4F6CIGQ" + "&field1=" + alarm +"&field2="+ String(temperature) + "&field3=" + l  + "&field4=" + f;
				  //String get = GET + writeApiKey + "&field1=" + alarm +"&field2="+ String(temperature) + "&field3=" + l  + "&field4=" + f;
				  String get = "http://" + String(serverName) + "/update?key=" + writeApiKey + "&field1=" + alarm + "&field2=" + String(level) +  "&field3="+ v1  + "&field4=" + v2 + "&field5=" + millis() +  "&field6=" + errorCounter +  "&field7=" + httpCode;
				  Serial.println(get);
#ifndef ESP8266
				  drawMessage(&display, "CONN TS ...");
#endif
				  //TODO:
				  http.begin(get);  //Specify request destination
				  httpCode = http.GET();                                                                  //Send the request
				  Serial.println(httpCode);
				  Serial.println(http.errorToString(httpCode));
#ifndef ESP8266
				  drawMessage(&display, "TS DONE");
#endif
				  if(httpCode != 200) {
					  errorCounter++;
					  errorConn = true;
				  }
				  else {
					  errorConn = false;
				  }

				  if (httpCode > 0) { //Check the returning code
					String payload = http.getString();   //Get the request response payload
					Serial.println(payload);                     //Print the response payload
				  }

				  http.end();   //Close connection
			  }
#endif

		 }

		 delay(60000);

		 if(isAP && reconnectTimeout > 10) {
			 reconnectTimeout = 0;

			 //TODO: auto reconnect
#ifndef ESP8266
			 //drawMessage(&display, "C " + String(millis()));
			 //drawMessage(&display, "CONN WIFI ... " + String(millis()));
#endif
			 //WiFi.softAPdisconnect(true);
			 WiFi.mode(WIFI_STA);
			 WiFi.begin();
			 WiFi.mode(WIFI_STA);
			 //WiFi.disconnect(true);
			 WiFiManager wifiManager;
			 //wifiManager.resetSettings();
			 for(int i = 0; i < 3; i++) {
#ifndef ESP8266
			  	 drawMessage(&display, "CONN WIFI ... " + String(i));
#endif
				 if(wifiManager.autoConnect()) {
					  isAP = false;
					  Serial.print("IP address: ");
					  Serial.println(WiFi.localIP());
#ifndef ESP8266
					  drawMessage(&display, "CONN WIFI");
					  break;
				 }
#endif
   			  }
			  if(isAP) {
#ifndef ESP8266
				  //drawMessage(&display, "F " + String(millis()));
				  drawMessage(&display, "CONN WIFI FAIL");
#endif
				  startWiFiAP();
				  isAP = true;
			 }
		 }
		 else
			 reconnectTimeout++;
	 }
}
void loop() {

#ifdef ESP8266
	loop1(0);
#endif



   if(devices[DEV_LEV_CAL].par4 - devices[DEV_LEV_CAL].par3) {
	   k = (float)(devices[DEV_LEV_CAL].par4 - devices[DEV_LEV_CAL].par3) / (float)(devices[DEV_LEV_CAL].par2 - devices[DEV_LEV_CAL].par1);
	   d = (float)devices[DEV_LEV_CAL].par3;
   }
   else {
	   k = 1.0;
	   d = 0.0;
   }
   level = analogRead(A0, SAMPLES) * k + d; //1/4095 * analogRead(A0) ;

	//int remainingTimeBudget = ui.update();
	//if (remainingTimeBudget > 0) {
		// You can do some work here
		// Don't do stuff if you are below your
		// time budget.
	//	delay(remainingTimeBudget);
		//ui.nextFrame();
	//}

	bool alarm = bitRead(devices[DEV_ALARM_MAX].flags, OUTPUT_BIT) | bitRead(devices[DEV_ALARM_MIN].flags, OUTPUT_BIT);
	bool unack = bitRead(devices[DEV_ALARM_MAX].flags, UNACK_BIT) | bitRead(devices[DEV_ALARM_MIN].flags, UNACK_BIT);

	//String s = String(touchRead(T4)) + " " + String(touchRead(T5)) + " " +String(touchRead(T6)) + " " +String(touchRead(T7)) + " " + String(touchRead(T8));
	//drawMessage(&display, s);


	if(!digitalRead(CONFIG_WIFI_PIN)) {
		bitClear(devices[DEV_ALARM_MAX].flags, UNACK_BIT);
		bitClear(devices[DEV_ALARM_MIN].flags, UNACK_BIT);
#ifndef ESP8266
		display.normalDisplay();
#endif
		//frameWait = 2;
	}
	/*if(frameWait > 0) {
		ui.disableAutoTransition();
	}
	else {
		ui.enableAutoTransition();
	}*/

	// put your main code here, to run repeatedly:



  // wdt test
  //ESP.wdtDisable(); //disable sw wdt, hw wdt keeps on
  //while(1){};

  ArduinoOTA.handle();
  server.handleClient();
  client.loop();

#ifdef LCD
  display.clearDisplay();
  display.setCursor(0,0);
  display.println(WiFi.localIP());
  display.println(timeClient.getFormattedTime());
  display.print(temperature);
  display.print('C');
  display.display();
#endif

  if(minInterval.expired()) {
	  minInterval.set(60000);
	  //minInterval.set(5000);

#ifdef LED1_PIN
	  digitalWrite(LED1_PIN, LOW);
#endif
/*
	  //if(!isAP)
	  if(false) {

		  drawMessage(&display, "CONN NTP ...");
		  timeClient.update();
		  drawMessage(&display, "NTP DONE");

	  if(!checkin) {
		  drawMessage(&display, "CONN GM ...");
		  http.begin("http://growmat.cz/growmatweb/dev/");
	  	  httpCode = http.GET();
	  	  if(httpCode > 0)
	  		checkin = true;
	  	drawMessage(&display, "GM DONE");
	  }

	  if(serverName[0] != 0 && !isAP) {

		  //HTTPClient http;  //Declare an object of class HTTPClient

		  //int httpCode;
		  do {
			  //http.begin("http://api.thingspeak.com/talkbacks/18221/commands/execute?api_key=ED57BAC7V0GLPWL2");
			  //String getTalkback = "http://api.thingspeak.com/talkbacks/" + String(talkbackID) + "/commands/execute?api_key=" + talkbackApiKey;

			  drawMessage(&display, "CONN TB ...");
			  String getTalkback = "http://" + String(serverName) + "/talkbacks/" + String(talkbackID) + "/commands/execute?api_key=" + talkbackApiKey;
			  Serial.println(getTalkback);

			  //TODO:
			  http.begin(getTalkback);
			  httpCode = http.GET();                                                                  //Send the request
			  Serial.println(httpCode);
			  Serial.println(http.errorToString(httpCode));


			  drawMessage(&display, "TB DONE");



			  if (httpCode > 0) { //Check the returning code
				  String payload = http.getString();   //Get the request response payload
				  Serial.println(payload);                     //Pri
				  if(payload == "")
					  break;
				  if(payload == "error_auth_required")
					  break;
				  if(payload.charAt(0)=='A') {
					  if(payload.charAt(1)=='0') {
						  bitClear(devices[0].flags, OUTPUT_BIT);
						  bitSet(devices[0].flags, MANUAL_BIT);
					  }
					  else if(payload.charAt(1)=='1') {
						  bitSet(devices[0].flags, OUTPUT_BIT);
						  bitSet(devices[0].flags, MANUAL_BIT);
					  }
					  else if(payload.charAt(1)=='A') {
						  bitClear(devices[0].flags, MANUAL_BIT);
					  }
				  }
				  if(payload.charAt(0)=='B') {
					  if(payload.charAt(1)=='0') {
						  bitClear(devices[1].flags, OUTPUT_BIT);
						  bitSet(devices[1].flags, MANUAL_BIT);
					  }
					  else if(payload.charAt(1)=='1') {
						  bitSet(devices[1].flags, OUTPUT_BIT);
						  bitSet(devices[1].flags, MANUAL_BIT);
					  }
					  else if(payload.charAt(1)=='A') {
						  bitClear(devices[1].flags, MANUAL_BIT);
					  }
				  }
			  }
			  else
				  break;
		  } while (true);


		  String alarm = String(bitRead(devices[3].flags, OUTPUT_BIT) * 1 + bitRead(devices[4].flags, OUTPUT_BIT) * 10);
		  String v1 = String(bitRead(devices[0].flags, OUTPUT_BIT) + bitRead(devices[0].flags, MANUAL_BIT) * 10);
		  String v2 = String(bitRead(devices[1].flags, OUTPUT_BIT) + bitRead(devices[1].flags, MANUAL_BIT) * 10);
		  String get = "http://" + String(serverName) + "/update?key=" + writeApiKey + "&field1=" + alarm + "&field2=" + String(level) + "&field3=" + v1 + "&field4=" + v2 + "&field5="  + millis() +  "&field6=" + errorCounter +  "&field7=" + httpCode;
		  Serial.println(get);


		  drawMessage(&display, "CONN TS ...");

		  //TODO:
		  http.begin(get);  //Specify request destination
		  httpCode = http.GET();                                                                  //Send the request
		  Serial.println(httpCode);
		  Serial.println(http.errorToString(httpCode));


		  drawMessage(&display, "TS DONE");


		  if(httpCode != 200)
			  errorCounter++;

		  if (httpCode > 0) { //Check the returning code
			String payload = http.getString();   //Get the request response payload
			Serial.println(payload);                     //Print the response payload
		  }

		  http.end();   //Close connection
  	  }
	  }
*/

#ifdef LED1_PIN
	  digitalWrite(LED1_PIN, HIGH);
#endif

  }



  if (secInterval.expired()) {

  		secInterval.set(1000);

#ifndef ESP8266
  		if(unack && alarm) {
  			if(secCounter % 2)
  				display.invertDisplay();
  			else
  				display.normalDisplay();
  		}


  		if(digitalRead(CONFIG_WIFI_PIN)==HIGH) {
  			frameNo++;
  		}
  		if(frameNo >= frameCount)
  			frameNo = 0;
  		drawDisplay(&display, frameNo);
#endif
  		//if(frameWait) {
  		//	frameWait--;
  		//}

  		if(isAP) {
#ifdef LED1_PIN
  			digitalWrite(LED1_PIN, led1);
#endif
  			led1 = !led1;
  		}

  		if(secCounter == 0)
			secOverflow = true;

  		if((secCounter & 0xF) == 0xF) {
  			//drawMessage(&display, "SENSORS ...");

			//TODO:
			//drawMessage(&display, "SENSORS DONE");
  		}

#ifdef RFRX_PIN
		if (rcSwitch.available()) {
			Serial.print(rcSwitch.getReceivedValue());
			Serial.print('\t');
			Serial.print(rcSwitch.getReceivedBitlength());
			Serial.print('\t');
			Serial.print(rcSwitch.getReceivedDelay());
			Serial.print('\t');
			unsigned int* p = rcSwitch.getReceivedRawdata();
			for(int i = 0; i < RCSWITCH_MAX_CHANGES; i++)
				Serial.print(*(p + i));
			Serial.print('\t');
			Serial.print(rcSwitch.getReceivedProtocol());
		    rcSwitch.resetAvailable();
		    Serial.println();
		 }
#endif
/*

		if(bitRead(devices[0].flags, OUTPUT_BIT)) {
			if(lightOnSecCounter < -1)
					lightOnSecCounter++;
			lightOffSecCounter = 0;
		}
		else {
			if(lightOffSecCounter < -1)
				lightOffSecCounter++;
			lightOnSecCounter = 0;
		}*/
  		//Serial.print('\n');
  		for(int i = 0; i < DEVICES_NUM; i++) {
			if(i < DEV_ALARM_MAX) {
				unsigned int onSec = devices[i].par1 * 3600 + devices[i].par2 * 60;
				unsigned int offSec = devices[i].par3 * 60 + devices[i].par4;

				//unsigned int onSec = timeClient.getHours() * 3600 + timeClient.getMinutes() * 60;
				time_t t = CE.toLocal(timeClient.getEpochTime());
				byte h = (t / 3600) % 24;
				byte m = (t / 60) % 60;
				byte s = t % 60;
				unsigned int sec = h * 3600 + m * 60;

				if(onSec) {
					if(onSec == sec) {
						Serial.println(String(i) + ": onTime");
						bitClear(devices[i].flags, MANUAL_BIT);
						bitSet(devices[i].flags, OUTPUT_BIT);
						//valveOpenSecCounters[i] = 0;
					}
				}

				//TODO: run once
			    byte levelOn = 0;
				//if(i == 0)
					levelOn = devices[DEV_ALARM_MAX].par1;
				//if(i == 1)
				//	levelOn = devices[4].par2;
				if(!bitRead(devices[i].flags, RUNONCE_BIT)) {
					if((level > levelOn) && levelOn) {
						Serial.println(String(i) + ": onLevel");
						bitSet(devices[i].flags, RUNONCE_BIT);
						bitClear(devices[i].flags, MANUAL_BIT);
						bitSet(devices[i].flags, OUTPUT_BIT);
						valveOpenSecCounters[i] = 0;
					}
				}

				if(offSec)
					if(valveOpenSecCounters[i] > offSec) {
						Serial.println(String(i) + ": offTime");
						bitClear(devices[i].flags, MANUAL_BIT);
						bitClear(devices[i].flags, OUTPUT_BIT);
						valveOpenSecCounters[i] = 0;
			  		}
				}

				byte levelOff = 0;
				//if(i == 0)
					levelOff = devices[DEV_ALARM_MIN].par1;
				//if(i == 1)
				//	levelOff = devices[3].par2;
				if((level <= levelOff) && levelOff) {
					Serial.println(String(i) + ": offLevel");

					bitClear(devices[i].flags, RUNONCE_BIT);
					bitClear(devices[i].flags, MANUAL_BIT);
					bitClear(devices[i].flags, OUTPUT_BIT);
					valveOpenSecCounters[i] = 0;
				}
				if(i == DEV_ALARM_MAX) {
					if(level >= devices[i].par3 && devices[i].par3) {
						if(!bitRead(devices[i].flags, OUTPUT_BIT))
							bitSet(devices[i].flags, UNACK_BIT);
						bitSet(devices[i].flags, OUTPUT_BIT);
						}
					if((level <= (devices[i].par3 - devices[i].par4)) || !devices[i].par3)
						bitClear(devices[i].flags, OUTPUT_BIT);
				}

				if(i==DEV_ALARM_MIN) {
					if(level <= devices[i].par3 && devices[i].par3) {
						if(!bitRead(devices[i].flags, OUTPUT_BIT))
							bitSet(devices[i].flags, UNACK_BIT);
						bitSet(devices[i].flags, OUTPUT_BIT);
						}
					if(level >= ((devices[i].par3 - devices[i].par4)) || !devices[i].par3)
						bitClear(devices[i].flags, OUTPUT_BIT);
				}


  		/*
			if(i==1) {
				if(!bitRead(devices[1].flags, MANUAL_BIT)) {
					if(bitRead(devices[0].flags, OUTPUT_BIT)) {
						//light
						if(temperature >= devices[1].par1)
							bitSet(devices[1].flags, OUTPUT_BIT);
						if(temperature <= devices[1].par2)
							bitClear(devices[1].flags, OUTPUT_BIT);
					}
					else {
						//dark
						if(temperature >= devices[1].par3)
							bitSet(devices[1].flags, OUTPUT_BIT);
						if(temperature <= devices[1].par4)
							bitClear(devices[1].flags, OUTPUT_BIT);
					}
				}

			}

			if(i==2) {
				//TODO: cycler
				if(!bitRead(devices[2].flags, OUTPUT_BIT)) {
					if(cyclerSecCounter > devices[2].par1 * 60 + devices[2].par2) {
						bitSet(devices[2].flags, OUTPUT_BIT);
						cyclerSecCounter = 0;
					}
				}
				else {
					if(cyclerSecCounter > devices[2].par3 * 60 + devices[2].par4) {
						bitClear(devices[2].flags, OUTPUT_BIT);
						cyclerSecCounter = 0;
					}
				}
				cyclerSecCounter++;
			}
			if(i==3) {
				if(!bitRead(devices[0].flags, OUTPUT_BIT) && lightOffSecCounter > devices[3].par2) {
					//dark
					if(analogIn >= devices[3].par1) {
						if(!bitRead(devices[3].flags, OUTPUT_BIT))
							bitSet(devices[3].flags, UNACK_BIT);
						bitSet(devices[3].flags, OUTPUT_BIT);
					}
					if(analogIn <= devices[3].par1 - 5.0)
						bitClear(devices[3].flags, OUTPUT_BIT);
				}
				else {
					//light
					bitClear(devices[3].flags, OUTPUT_BIT);
				}
			}
			if(i==4) {
				if(bitRead(devices[0].flags, OUTPUT_BIT && lightOnSecCounter > devices[4].par2)) {
					//light
					if(analogIn <= devices[4].par1) {
						if(!bitRead(devices[4].flags, OUTPUT_BIT))
							bitSet(devices[4].flags, UNACK_BIT);
						bitSet(devices[4].flags, OUTPUT_BIT);
					}
					if(analogIn >= devices[4].par1 + 5.0)
						bitClear(devices[4].flags, OUTPUT_BIT);
				}
				else {
					//dark
					bitClear(devices[4].flags, OUTPUT_BIT);
				}
			}

			if(i==5) {
				if(bitRead(devices[0].flags, OUTPUT_BIT) || lightOnSecCounter > devices[5].par3) {
					//light
					if(temperature >= devices[5].par1) {
						if(!bitRead(devices[5].flags, OUTPUT_BIT))
							bitSet(devices[5].flags, UNACK_BIT);
						bitSet(devices[5].flags, OUTPUT_BIT);
					}
					if(temperature <= devices[5].par1 - 0.5)
						bitClear(devices[5].flags, OUTPUT_BIT);
				}
				else {
					//dark
					if(temperature >= devices[5].par2) {
						if(!bitRead(devices[5].flags, OUTPUT_BIT))
							bitSet(devices[5].flags, UNACK_BIT);
						bitSet(devices[5].flags, OUTPUT_BIT);
					}
					if(temperature <= devices[5].par2 - 0.5)
						bitClear(devices[5].flags, OUTPUT_BIT);
				}
			}
			if(i==6) {
				if(bitRead(devices[0].flags, OUTPUT_BIT) || lightOffSecCounter < devices[5].par3) {
					//light
					if(temperature <= devices[6].par1) {
						if(!bitRead(devices[6].flags, OUTPUT_BIT))
							bitSet(devices[6].flags, UNACK_BIT);
						bitSet(devices[6].flags, OUTPUT_BIT);
					}
					if(temperature >= devices[6].par1 + 0.5)
						bitClear(devices[6].flags, OUTPUT_BIT);
				}
				else {
					//dark
					if(temperature <= devices[6].par2) {
						if(!bitRead(devices[6].flags, OUTPUT_BIT))
							bitSet(devices[6].flags, UNACK_BIT);
						bitSet(devices[6].flags, OUTPUT_BIT);
					}
					if(temperature >= devices[6].par2 + 0.5)
						bitClear(devices[6].flags, OUTPUT_BIT);
				}

			}
*/
  			//Serial.print(devices[i].flags);
  			//Serial.print('\t');
  			//Serial.print(devices[i].name);
  			//Serial.print('\n');


  		}

  		if((secCounter & 0xF) == 0xF) {
#ifdef RFTX_PIN
  			//light protection
  			if(secCounter > devices[7].par1 || secOverflow || bitRead(devices[0].flags, MANUAL_BIT)) {
				if(bitRead(devices[0].flags, OUTPUT_BIT))
					//sendSignal(RFTX_PIN, rf1on);
					rcSwitch.send(r1On, 24);
				else
					//sendSignal(RFTX_PIN, rf1off);
					rcSwitch.send(r1Off, 24);
			}
			if(bitRead(devices[1].flags, OUTPUT_BIT))
				rcSwitch.send(r2On, 24);
			else
				rcSwitch.send(r2Off, 24);
			if(bitRead(devices[2].flags, OUTPUT_BIT))
				rcSwitch.send(r3On, 24);
			else
				rcSwitch.send(r3Off, 24);
#endif
			//secCounter = 0;
  		}
  		secCounter++;

  		for(int i = 0; i < DEV_ALARM_MAX; i++) {
			if(bitRead(devices[i].flags, OUTPUT_BIT)) {
				valveOpenSecCounters[i]++;
			}
  		}
  		//if(bitRead(devices[1].flags, OUTPUT_BIT)) {
  		//	valveOpenSecCounters[1]++;
  		//}

#ifdef LED0_PIN
		digitalWrite(LED0_PIN, not(bitRead(devices[DEV_ALARM_MAX].flags, OUTPUT_BIT) | bitRead(devices[DEV_ALARM_MIN].flags, OUTPUT_BIT)));
#endif

		if(digitalRead(CONFIG_WIFI_PIN) == LOW) {
			if(bitRead(devices[0].flags, OUTPUT_BIT)) {
				//Serial.println("0: onButton");
				bitClear(devices[0].flags, MANUAL_BIT);
				bitClear(devices[0].flags, OUTPUT_BIT);
			}
			else {
				bitSet(devices[0].flags, MANUAL_BIT);
				bitSet(devices[0].flags, OUTPUT_BIT);
			}
		}
		if(digitalRead(INPUT1_PIN) == LOW) {
			if(bitRead(devices[1].flags, OUTPUT_BIT)) {
				bitClear(devices[1].flags, MANUAL_BIT);
				bitClear(devices[1].flags, OUTPUT_BIT);
			}
			else {
				bitSet(devices[1].flags, MANUAL_BIT);
				bitSet(devices[1].flags, OUTPUT_BIT);
			}
		}
		if(digitalRead(INPUT2_PIN) == LOW) {
			if(bitRead(devices[2].flags, OUTPUT_BIT)) {
				bitClear(devices[2].flags, MANUAL_BIT);
				bitClear(devices[2].flags, OUTPUT_BIT);
			}
			else {
				bitSet(devices[2].flags, MANUAL_BIT);
				bitSet(devices[2].flags, OUTPUT_BIT);
			}
		}
		if(digitalRead(INPUT3_PIN) == LOW) {
			if(bitRead(devices[3].flags, OUTPUT_BIT)) {
				bitClear(devices[3].flags, MANUAL_BIT);
				bitClear(devices[3].flags, OUTPUT_BIT);
			}
			else {
				bitSet(devices[3].flags, MANUAL_BIT);
				bitSet(devices[3].flags, OUTPUT_BIT);
			}
		}

#ifdef OUTPUT0_PIN

		digitalWrite(OUTPUT0_PIN, not(bitRead(devices[0].flags, OUTPUT_BIT))); //| (digitalRead(CONFIG_WIFI_PIN) == LOW)));
		digitalWrite(OUTPUT1_PIN, not(bitRead(devices[1].flags, OUTPUT_BIT))); //| (digitalRead(INPUT1_PIN) == LOW)));
		digitalWrite(OUTPUT2_PIN, not(bitRead(devices[2].flags, OUTPUT_BIT))); //| (digitalRead(INPUT1_PIN) == LOW)));
		digitalWrite(OUTPUT3_PIN, not(bitRead(devices[3].flags, OUTPUT_BIT)));
		//digitalWrite(OUTPUT3_PIN, not(bitRead(devices[3].flags, OUTPUT_BIT) | bitRead(devices[4].flags, OUTPUT_BIT)));
#endif
		if(!mqttLock.test_and_set() && mqttServer[0] != 0) {


			//if (!client.connected()) {
			//	mqttconnect();
			//}
			yield();
			if (client.connected()) {
				for(int i = 0; i < DEVICES_NUM; i++) {
					if(bitRead(devices[i].flags, OUTPUT_BIT) != bitRead(devices[i].flags, PREVOUTPUT_BIT)) {
						if(bitRead(devices[i].flags, OUTPUT_BIT))
							bitSet(devices[i].flags, PREVOUTPUT_BIT);
						else
							bitClear(devices[i].flags, PREVOUTPUT_BIT);
						snprintf (msg, 20, "%d", bitRead(devices[i].flags, OUTPUT_BIT));
						if(i == 0) {
							//client.publish(A_VAL_TOPIC, msg);
							client.publish(String(ROOT_TOPIC + String(mqttID) + A_VAL_TOPIC).c_str(), msg);
						}
						if(i == 1) {
							//client.publish(B_VAL_TOPIC, msg);
							client.publish(String(ROOT_TOPIC + String(mqttID) + B_VAL_TOPIC).c_str(), msg);
						}
						if(i == 2) {
							//client.publish(C_VAL_TOPIC, msg);
							client.publish(String(ROOT_TOPIC + String(mqttID) + C_VAL_TOPIC).c_str(), msg);
						}
						if(i == 3) {
							//client.publish(D_VAL_TOPIC, msg);
							client.publish(String(ROOT_TOPIC + String(mqttID) + D_VAL_TOPIC).c_str(), msg);
						}
						if(i == DEV_ALARM_MAX) {
							//client.publish(LEVEL_MAX_TOPIC, msg);
							client.publish(String(ROOT_TOPIC + String(mqttID) + LEVEL_MAX_TOPIC).c_str(), msg);
						}
						if(i == DEV_ALARM_MIN) {
							//client.publish(LEVEL_MIN_TOPIC, msg);
							client.publish(String(ROOT_TOPIC + String(mqttID) + LEVEL_MIN_TOPIC).c_str(), msg);
						}
					}
				}
			}
			mqttLock.clear();
		}
  }


	 snprintf (msg, 20, "%d", bitRead(devices[0].flags, OUTPUT_BIT));
	 //client.publish(A_VAL_TOPIC, msg);
	 client.publish(String(String(mqttID) + A_VAL_TOPIC).c_str(), msg);

	 snprintf (msg, 20, "%d", bitRead(devices[1].flags, OUTPUT_BIT));
	 //client.publish(B_VAL_TOPIC, msg);
	 client.publish(String(String(mqttID) + B_VAL_TOPIC).c_str(), msg);

	 snprintf (msg, 20, "%d", bitRead(devices[2].flags, OUTPUT_BIT));
	 //client.publish(C_VAL_TOPIC, msg);
	 client.publish(String(String(mqttID) + C_VAL_TOPIC).c_str(), msg);

	 snprintf (msg, 20, "%d", bitRead(devices[3].flags, OUTPUT_BIT));
	 //client.publish(D_VAL_TOPIC, msg);
	 client.publish(String(String(mqttID) + D_VAL_TOPIC).c_str(), msg);




  //get a random server from the pool
  //WiFi.hostByName(ntpServerName, timeServerIP);

  //sendNTPpacket(timeServerIP); // send an NTP packet to a time server
   // wait to see if a reply is available
   //delay(1000);
/*
   int cb = udp.parsePacket();
   if (!cb) {
     Serial.println("no packet yet");
   }
   else {
     Serial.print("packet received, length=");
     Serial.println(cb);
     // We've received a packet, read the data from it
     udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

     //the timestamp starts at byte 40 of the received packet and is four bytes,
     // or two words, long. First, esxtract the two words:

     unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
     unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
     // combine the four bytes (two words) into a long integer
     // this is NTP time (seconds since Jan 1 1900):
     unsigned long secsSince1900 = highWord << 16 | lowWord;
     Serial.print("Seconds since Jan 1 1900 = " );
     Serial.println(secsSince1900);

     // now convert NTP time into everyday time:
     Serial.print("Unix time = ");
     // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
     const unsigned long seventyYears = 2208988800UL;
     // subtract seventy years:
     unsigned long epoch = secsSince1900 - seventyYears;
     // print Unix time:
     Serial.println(epoch);


     // print the par1, par2 and second:
     Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
     Serial.print((epoch  % 86400L) / 3600); // print the par1 (86400 equals secs per day)
     Serial.print(':');
     if ( ((epoch % 3600) / 60) < 10 ) {
       // In the first 10 par2s of each par1, we'll want a leading '0'
       Serial.print('0');
     }
     Serial.print((epoch  % 3600) / 60); // print the par2 (3600 equals secs per par2)
     Serial.print(':');
     if ( (epoch % 60) < 10 ) {
       // In the first 10 seconds of each par2, we'll want a leading '0'
       Serial.print('0');
     }
     Serial.println(epoch % 60); // print the second
   }
   // wait ten seconds before asking for the time again
   delay(10000);*/
}

/*
// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
  Serial.println("Sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}
*/

