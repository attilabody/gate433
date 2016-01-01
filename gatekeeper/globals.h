/*
 * globals.h
 *
 *  Created on: Dec 29, 2015
 *      Author: compi
 */

#ifndef GLOBALS_H_
#define GLOBALS_H_
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "intdb.h"
#include "inductiveloop.h"
#include "trafficlights.h"


extern const uint8_t	g_innerlightspins[3];
extern const uint8_t	g_outerlightspins[3];
extern const uint8_t	g_otherrelaypins[2];

extern LiquidCrystal_I2C	g_lcd;
extern intdb				g_db;
extern bool					g_dbinitfail;
extern trafficlights		g_lights;
extern inductiveloop		g_indloop;

#endif	//	GLOBALS_H_
