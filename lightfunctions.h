/*
 * lightfunctions.h
 *
 *  Created on: 06.04.2018
 *      Author: bened
 */

#ifndef LIGHTFUNCTIONS_H_
#define LIGHTFUNCTIONS_H_
#include "settings.h"
#include "FastLED.h"

extern CRGB leds[NUM_LEDS];

void runningLight(void);
void rainbow_strip(int offset);
uint8_t getcolorspectrum(int pos, int min, int max, String color, float freq_r,
		float freq_g, float freq_b);
void setSolid(int r, int g, int b);

#endif /* LIGHTFUNCTIONS_H_ */
