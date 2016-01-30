/*
 * globals.cpp
 *
 *  Created on: Dec 29, 2015
 *      Author: compi
 */

#include "globals.h"
#include <ds3231.h>
#include <PCF8574.h>
#include "display.h"
#include "sdfatlogwriter.h"

const uint8_t	g_otherrelaypins[2] = { PIN_GATE, PIN_RELAY_SPARE };

display 			g_display(LCD_I2C_ADDRESS, LCD_WIDTH, LCD_HEIGHT);
SdFat				g_sd;
#ifdef USE_THINDB
thindb				g_db( g_sd );
#endif	//	USE_THINDB
#ifdef USE_INTDB
intdb				g_db( g_sd );
#endif	//	USE_INTDB
#ifdef USE_HYBRIDDB
hybriddb			g_db( g_sd, HYBRIDDB_EEPROM_ADDRESS);
#endif	//	USE_HYBRIDDB
#ifdef USE_FLASHDB
flashdb				g_db( FLASHDB_EEPROM_ADDRESS, FLASHDB_EEPROM_BITS, 128 );
#endif	//	USE_FLASHDB
inductiveloop		g_indloop;
PCF8574				g_i2cio( PCF8574_ADDRESS );

char				g_iobuf[32];
uint8_t				g_inidx(0);

uint16_t			g_codedisplayed((uint16_t)-1);

bool				g_sdpresent(false);

ts					g_t;
unsigned long		g_lastdtupdate(0);

sdfatlogwriter		g_logger( g_sd );
