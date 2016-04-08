/*
 * globals.cpp
 *
 *  Created on: Dec 29, 2015
 *      Author: compi
 */

#include "globals.h"
#include <ds3231.h>
#include "display.h"
#include "sdfatlogwriter.h"

display 				g_display(LCD_I2C_ADDRESS, LCD_WIDTH, LCD_HEIGHT);
SdFat					g_sd;
#ifdef USE_THINDB
thindb					g_db( g_sd );
#endif	//	USE_THINDB
#ifdef USE_INTDB
intdb					g_db( g_sd );
#endif	//	USE_INTDB
#ifdef USE_HYBRIDDB
hybriddb				g_db( g_sd, HYBRIDDB_EEPROM_ADDRESS);
#endif	//	USE_HYBRIDDB
#ifdef USE_HYBINTDB
hybintdb				g_db( g_sd );
#endif	//	USE_HYBINTDB
#ifdef USE_I2CDB
i2cdb					g_db( I2CDB_EEPROM_ADDRESS, I2CDB_EEPROM_BITS, 128 );
#endif	//	USE_FLASHDB
inductiveloop			g_indloop;

#ifdef USE_IOEXTENDER_OUTPUTS
PCF8574outputs			g_outputs(PCF8574_ADDRESS);
#else	//	USE_IOEXTENDER_OUTPUTS
arduinooutputs			g_outputs;
#endif	//	USE_IOEXTENDER_OUTPUTS

trafficlights			g_lights;
char					g_iobuf[32];
uint8_t					g_inidx(0);

uint16_t				g_codedisplayed((uint16_t)-1);

bool					g_sdpresent(false);

DS3231_DST				g_clk;
ts						g_time;
bool					g_timevalid(false);
unsigned long			g_lastdtupdate(0);

sdfatlogwriter			g_logger( g_sd );

uint16_t				g_lastcheckpoint(0);
