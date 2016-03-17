/*
 * config.h
 *
 *  Created on: Feb 6, 2016
 *      Author: compi
 */

#ifndef CONFIG_H_
#define CONFIG_H_
#include <Arduino.h>

#define BAUDRATE 115200
#define ENFORCE
//#define DECODE433_REVERSE
//#define VERBOSE
#define USE_I2CDB
//#define	USE_EEPROMDB
#define USE_IOEXTENDER_OUTPUTS

#define I2CDB_EEPROM_ADDRESS	0x50
#define I2CDB_EEPROM_BITS		16
#define	I2CDB_EEPROMPAGE_LENGTH	128
#define I2CDB_EEPROM_OFFSET		0

#define LCD_I2C_ADDRESS	0x27	//	usually 0x27 or 0x3f
#define LCD_WIDTH	16		//	in characters
#define LCD_HEIGHT	2		//	in lines

#define PIN_IN_GREEN		0
#define PIN_IN_YELLOW		1
#define PIN_IN_RED			2
#define PIN_OUT_GREEN		3
#define PIN_OUT_YELLOW		7
#define	PIN_OUT_RED			6
#define	PIN_RELAY_SPARE		5
#define PIN_GATE			4

#define INNER_LIGHTS_PINS 	PIN_IN_GREEN,PIN_IN_YELLOW,PIN_IN_RED
#define OUTER_LIGHTS_PINS 	PIN_OUT_GREEN,PIN_OUT_YELLOW,PIN_OUT_RED
#define OTHER_RELAY_PINS	PIN_GATE,PIN_RELAY_SPARE

#ifdef USE_IOEXTENDER_OUTPUTS	//	PCF8574 outputs
#define PCF8574_ADDRESS		0x20
#else	//	USE_IOEXTENDER_OUTPUTS
#define	ALL_RAW_OUTPUT_PINS	9,8,7,6,A2,A3,4,5	//INNER_LIGHTS_PINS,OUTER_LIGHTS_PINS,OTHER_RELAY_PINS
#endif	//	USE_IOEXTENDER_OUTPUTS

#define PIN_RFIN		2
#define PIN_INNERLOOP	A0
#define PIN_OUTERLOOP	A1
#define LOOP_ACTIVE		LOW

#define RELAY_OFF	HIGH
#define RELAY_ON	LOW

#define ID_MIN	4
#define	ID_MAX	1019

#endif /* CONFIG_H_ */
