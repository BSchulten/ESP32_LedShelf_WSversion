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

//##########	Defs etc.
CRGB leds[NUM_LEDS];
#define DATA_PIN 18
#define interruptPin 33
#define debouncetime 100

volatile int interruptCounter;
volatile int btninterruptCounter;
int totalInterruptCounter;
int lastbtntoggle = 0;



hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

//#########		Creates

AsyncWebServer server(80);
AsyncWebSocket ws("/");
SSD1306 display(0x3c, 21, 22);

//##########	Global Vars

const char* ssid = "SSID";
const char* password = "PASSWORD";
const int LedPin = 17;
int currentColor = 0;
unsigned long mytime = 0;

int mode = 0;
int currentfade = 0;

//##################
// Prototypes
void displayConnection(void);
void handleMessage(AsyncWebSocketClient * client, uint8_t *rawdata, String msg);
// End of Prototypes
//###################

void IRAM_ATTR onTimer()
{
	portENTER_CRITICAL_ISR(&timerMux);
	interruptCounter++;
	portEXIT_CRITICAL_ISR(&timerMux);
}

void IRAM_ATTR handleInterrupt() {
  portENTER_CRITICAL_ISR(&mux);
  btninterruptCounter++;
  portEXIT_CRITICAL_ISR(&mux);
}

void setupWifi()
{
	WiFi.begin(ssid, password);
	display.clear();
	display.drawString(0, 0, "Connecting to:");
	display.drawString(0, 15, (String) ssid);
	display.drawString(0, 30, "PWD:");
	display.drawString(0, 45, (String) password);
	display.display();

	int i = 0;
	String dots = ".";

	while (WiFi.status() != WL_CONNECTED)
	{
		delay(1000);
		Serial.println("Connecting to WiFi..");
		display.drawString(80, 0, dots);
		i++;
		dots = dots + ".";
		display.display();
	}

	Serial.println("Connected to the WiFi network");
	Serial.println(WiFi.localIP());
	display.clear();
	displayConnection();
}

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client,
		AwsEventType type, void * arg, uint8_t *data, size_t len)
{
	if (type == WS_EVT_CONNECT)
	{
		Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
		client->printf("Hello Client %u :)", client->id());
		client->ping();
	}
	else if (type == WS_EVT_DISCONNECT)
	{
		Serial.printf("ws[%s][%u] disconnect: %u\n", server->url(),
				client->id());
	}
	else if (type == WS_EVT_ERROR)
	{
		Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(),
				*((uint16_t*) arg), (char*) data);
	}
	else if (type == WS_EVT_PONG)
	{
		Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(),
				len, (len) ? (char*) data : "");
	}
	else if (type == WS_EVT_DATA)
	{
		AwsFrameInfo * info = (AwsFrameInfo*) arg;
		String msg = "";
		if (info->final && info->index == 0 && info->len == len)
		{
			//the whole message is in a single frame and we got all of it's data
			Serial.printf("ws[%s][%u] %s-message[%llu]: ", server->url(),
					client->id(), (info->opcode == WS_TEXT) ? "text" : "binary",
					info->len);

			if (info->opcode == WS_TEXT)
			{
				for (size_t i = 0; i < info->len; i++)
				{
					msg += (char) data[i];
				}
			}
			else
			{
				char buff[3];
				for (size_t i = 0; i < info->len; i++)
				{
					sprintf(buff, "%02x ", (uint8_t) data[i]);
					msg += buff;
				}
			}
			Serial.printf("%s\n", msg.c_str());
			handleMessage(client, data, msg);

			/*if (info->opcode == WS_TEXT)
			 client->text("I got your text message");
			 else
			 client->binary("I got your binary message");*/
		}
		else
		{
			//message is comprised of multiple frames or the frame is split into multiple packets
			if (info->index == 0)
			{
				if (info->num == 0)
					Serial.printf("ws[%s][%u] %s-message start\n",
							server->url(), client->id(),
							(info->message_opcode == WS_TEXT) ?
									"text" : "binary");
				Serial.printf("ws[%s][%u] frame[%u] start[%llu]\n",
						server->url(), client->id(), info->num, info->len);
			}

			Serial.printf("ws[%s][%u] frame[%u] %s[%llu - %llu]: ",
					server->url(), client->id(), info->num,
					(info->message_opcode == WS_TEXT) ? "text" : "binary",
					info->index, info->index + len);

			if (info->opcode == WS_TEXT)
			{
				for (size_t i = 0; i < info->len; i++)
				{
					msg += (char) data[i];
				}
			}
			else
			{
				char buff[3];
				for (size_t i = 0; i < info->len; i++)
				{
					sprintf(buff, "%02x ", (uint8_t) data[i]);
					msg += buff;
				}
			}
			Serial.printf("%s\n", msg.c_str());

			if ((info->index + len) == info->len)
			{
				Serial.printf("ws[%s][%u] frame[%u] end[%llu]\n", server->url(),
						client->id(), info->num, info->len);
				if (info->final)
				{
					Serial.printf("ws[%s][%u] %s-message end\n", server->url(),
							client->id(),
							(info->message_opcode == WS_TEXT) ?
									"text" : "binary");
					if (info->message_opcode == WS_TEXT)
						client->text("I got your text message");
					else
						client->binary("I got your binary message");
				}
			}
		}
	}
}

void handleMessage(AsyncWebSocketClient * client, uint8_t *rawdata, String msg)
{
	client->text(msg);
	if (msg == "led_on")
	{
		digitalWrite(LedPin, HIGH);
	}
	else if (msg == "led_off")
	{
		digitalWrite(LedPin, LOW);
	}

	else if (msg == "rainbow")
	{
		mode = 1;
	}

	else if (msg == "off")
	{
		mode = 0;
	}

	else if (msg == "runner")
	{
		mode = 2;
	}

	else
	{
		if (millis() > (mytime + 30))
		{
			mode = 4;
			mytime = millis();
			int x;
			sscanf((const char*) rawdata, "%x", &x);
			Serial.println(x);
			displayConnection();
			display.drawString(0, 48, "Farbe: " + (msg));
			display.display();
			for (int i = 0; i < NUM_LEDS; i++)
			{
				leds[i].r = (x >> 16) & 0xFF;
				leds[i].g = (x >> 8) & 0xFF;
				leds[i].b = x & 0xFF;

			}
			FastLED.show();
		}
	}
	displayConnection();
	display.drawString(0, 48, "Farbe: " + (msg));
	display.drawString(100, 48, (String) mode);
	display.display();
}

void MoveRainbow(void)
{
	rainbow_strip(currentfade);
	currentfade++;
}

//###################################
//     Setup-Functions
void setupDisplay()
{
	display.init();
	display.flipScreenVertically();
	display.setFont(ArialMT_Plain_10);
	display.setColor(WHITE);
	display.setTextAlignment(TEXT_ALIGN_LEFT);
}

void displayConnection(void)
{
	display.clear();
	display.drawString(0, 0, "Connected to: ");
	display.drawString(0, 12, WiFi.SSID());
	display.drawString(0, 24, "with Local-IP:");
	display.drawString(0, 36, WiFi.localIP().toString());
	display.display();
}

void setupServer()
{
	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
	{
		request->send(SPIFFS, "/site.html");
	});
	server.serveStatic("/stylesheet.css", SPIFFS, "/stylesheet.css");
	server.serveStatic("/script.js", SPIFFS, "/script.js");
	server.serveStatic("/jscolor.js", SPIFFS, "/jscolor.js");
	//server.serveStatic("/", SPIFFS, "/");
	server.begin();
}

void setupTimer(void)
{
	timer = timerBegin(0, 80, true);
	timerAttachInterrupt(timer, &onTimer, true);
	timerAlarmWrite(timer, 30, true);
	timerAlarmEnable(timer);
}

void setup()
{
	//Serial.begin(115200);
	pinMode(LedPin, OUTPUT);
	digitalWrite(LedPin, LOW);
	pinMode(interruptPin, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, FALLING);

	setupDisplay();
	setupWifi();
	if (!SPIFFS.begin(0, "/spiffs", 10))
	{
		Serial.println("SPIFFS Mount Failed");
		return;
	}
	else
	{
		Serial.println("SPIFFS Mounted");
	}
	ws.onEvent(onWsEvent);
	server.addHandler(&ws);
	setupServer();
	setupTimer();
	FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
	FastLED.setBrightness(170);
	for (int i = 0; i < NUM_LEDS; i++)
	{
		leds[i] = 0x00FF00;
		//leds[i-1] = 0x000000;
		FastLED.show();
		FastLED.delay(1000 / 34);
	}
	delay(100);
}

//     End of Setup-Functions
//###################################

long lastFrame = 0;
void loop()
{
	FastLED.setBrightness(150);
	if (interruptCounter > 0 && mode != 3)
	{
		portENTER_CRITICAL(&timerMux);
		interruptCounter--;
		portEXIT_CRITICAL(&timerMux);
		long currentmillis = millis();
		long frameState = lastFrame + (1000 / frameRate);
		if (frameState <= currentmillis)
		{
			lastFrame = millis();
			if (mode == 1)
			{
				MoveRainbow();
			}

			else if (mode == 2)
			{
				runningLight();
			}

		}

	}
	if (btninterruptCounter > 0)
	{
		portENTER_CRITICAL(&mux);
		btninterruptCounter--;
		portEXIT_CRITICAL(&mux);
		int btntime = lastbtntoggle + debouncetime;
		if (btntime < millis())
		{
			if (mode != 3)
				mode = 3;
			else if (mode == 3)
				mode = 0;
		}

	}

	if (mode == 0)
	{
		setSolid(0, 0, 0);
	}

	else if (mode == 3)
	{
		setSolid(200, 200, 165);
	}

}
