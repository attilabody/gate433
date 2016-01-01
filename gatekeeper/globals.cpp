/*
 * globals.cpp
 *
 *  Created on: Dec 29, 2015
 *      Author: compi
 */

#include "globals.h"
#include "config.h"

const uint8_t	g_innerlightspins[3] = INNER_LIGHTS_PINS;
const uint8_t	g_outerlightspins[3] = OUTER_LIGHTS_PINS;
const uint8_t	g_otherrelaypins[2] = { PIN_GATE, PIN_RELAY_SPARE };

LiquidCrystal_I2C 	g_lcd(LCD_ADDRESS, LCD_WIDTH, LCD_HEIGHT);
intdb				g_db;
bool				g_dbinitfail;
trafficlights		g_lights;
inductiveloop		g_indloop;
