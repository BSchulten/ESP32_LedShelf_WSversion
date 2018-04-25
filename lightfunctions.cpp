/*
 * lightfunctions.c
 *
 *  Created on: 06.04.2018
 *      Author: bened
 */
#include "lightfunctions.h" //Include Header of current file

extern CRGB leds[NUM_LEDS]; //leds is created in Main file

unsigned int currentLightPos = 0;
const double Pi = 3.141592653;

void runningLight(void)
{
	if (currentLightPos < (NUM_LEDS - 1))
	{
		currentLightPos++;
	}
	else
	{
		currentLightPos = 0;
	}

	for (int i = 0; i < NUM_LEDS; i++)
	{
		leds[i] = 0x000000;
		leds[currentLightPos] = 0xFFFFFF;
	}

	FastLED.show();
}

void rainbow_strip(int offset)
{
	for (int i = 0; i < NUM_LEDS; i++)
	{
		leds[i].r = getcolorspectrum(i + offset, 0, NUM_LEDS, "r", 0.4, 0.4, 0.4);
		leds[i].g = getcolorspectrum(i + offset, 0, NUM_LEDS, "g", 0.4, 0.4, 0.4);
		leds[i].b = getcolorspectrum(i + offset, 0, NUM_LEDS, "b", 0.4, 0.4, 0.4);
	}
	FastLED.show();

}

uint8_t getcolorspectrum(int pos, int min, int max, String color, float freq_r,
		float freq_g, float freq_b)
{
	double relPos = ((double) pos) / ((double) (max - min));
	int colorVal = 0;
	int center = 128;
	double rad = ((relPos * 360) * Pi) / 180;
	if (color == "r")
	{
		colorVal = ((sin(freq_r * rad + 0)) * 127) + center;
		return colorVal;
	}
	else if (color == "g")
	{
		colorVal = ((sin(freq_g * rad + 2.4)) * 127) + center;
		return colorVal;
	}
	else if (color == "b")
	{
		colorVal = ((sin(freq_b * rad + 4.7)) * 127) + center;
		return colorVal;
	}
	else
	{
		return 0;
	}
}

void setSolid(int r, int g, int b){
	for (int i = 0; i < NUM_LEDS; i++)
				{
					leds[i].r = r;
					leds[i].g = g;
					leds[i].b = b;

				}
				FastLED.show();
}

