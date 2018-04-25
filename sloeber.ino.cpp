#ifdef __IN_ECLIPSE__
//This is a automatic generated file
//Please do not modify this file
//If you touch this file your change will be overwritten during the next build
//This file has been generated on 2018-04-25 15:29:52

#include "Arduino.h"
#include "settings.h"
#include <WiFi.h>
#include "SSD1306.h"
#include "FS.h"
#include <ESPmDNS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include <SPIFFSEditor.h>
#include <Wire.h>
#include "FastLED.h"
#include <math.h>
#include "lightfunctions.h"

void IRAM_ATTR onTimer() ;
void IRAM_ATTR handleInterrupt() ;
void setupWifi() ;
void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, 		AwsEventType type, void * arg, uint8_t *data, size_t len) ;
void handleMessage(AsyncWebSocketClient * client, uint8_t *rawdata, String msg) ;
void MoveRainbow(void) ;
void setupDisplay() ;
void displayConnection(void) ;
void setupServer() ;
void setupTimer(void) ;
void setup() ;
void loop() ;

#include "ESP32_LedShelf_WSversion.ino"


#endif
