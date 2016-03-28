/*
 * globals.cpp
 *
 *  Created on: Feb 8, 2016
 *      Author: abody
 */
#include <globals.h>
#include "config.h"

SdFat			g_sd;
sdfatlogwriter	g_logger( g_sd );
display 		g_display(LCD_I2C_ADDRESS, LCD_WIDTH, LCD_HEIGHT);

uint16_t		g_codedisplayed(-1);

#ifdef USE_I2CDB
i2cdb			g_db(I2CDB_EEPROM_ADDRESS, I2CDB_EEPROM_BITS, I2CDB_EEPROMPAGE_LENGTH);
#endif
#ifdef USE_EEPROMDB
eepromdb		g_db;
#endif

#ifdef USE_IOEXTENDER_OUTPUTS
PCF8574					g_outputs(PCF8574_ADDRESS);
#else	//	USE_IOEXTENDER_OUTPUTS
arduinooutputs			g_outputs;
#endif	//	USE_IOEXTENDER_OUTPUTS

DS3231_DST		g_clk;
inductiveloop	g_loop;

