/*
 * config.h
 *
 *  Created on: Nov 12, 2015
 *      Author: compi
 */

#ifndef CONFIG_H_
#define CONFIG_H_

//#define VERBOSE
//#define DBGSERIALIN
//#define ENFORCING

#define BAUDRATE 115200

//#define DECODE433_REVERSE

#define PIN_RFIN		2
#define PIN_INNERLOOP	A0
#define PIN_OUTERLOOP	A1
#define LOOP_ACTIVE		LOW

//PCF8574 outputs
#define PCF8574_ADDRESS		0x20
#define INNER_LIGHTS_PINS 	{ 0,1,2 }
#define OUTER_LIGHTS_PINS 	{ 3,7,6 }
#define	PIN_RELAY_SPARE		5
#define PIN_GATE			4

#define RELAY_OFF	HIGH
#define RELAY_ON	LOW

#define LCD_I2C_ADDRESS	0x27	//	usually 0x27 or 0x3f
#define LCD_WIDTH	16		//	in characters
#define LCD_HEIGHT	2		//	in lines

#define HYBRIDDB_EEPROM_OFFSET 0
#define HYBRIDDB_EEPROM_ADDRESS 0x57
#define I2CDB_EEPROM_OFFSET 0
#define I2CDB_EEPROM_ADDRESS 0x50
#define I2CDB_EEPROM_BITS 16

//#define USE_INTDB
//#define USE_THINDB
//#define USE_HYBRIDDB
#define USE_I2CDB

#endif /* CONFIG_H_ */
