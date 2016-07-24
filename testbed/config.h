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
#define USE_IOEXTENDER_OUTPUTS
#define BAUD 115200

#define PIN_RFIN		2
#define PIN_INNERLOOP	A0
#define PIN_OUTERLOOP	A1
#define LOOP_ACTIVE		LOW

#define PIN_RFIN		2
#define PIN_RESET		3
#define PIN_INNERLOOP	A0
#define PIN_OUTERLOOP	A1
#define LOOP_ACTIVE		LOW

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

#define PCF8574_ADDRESS		0x20
//for arduino outputs
#define	ALL_RAW_OUTPUT_PINS	9,8,7,6,A2,A3,4,5	//INNER_LIGHTS_PINS,OUTER_LIGHTS_PINS,OTHER_RELAY_PINS

#define RELAY_OFF	HIGH
#define RELAY_ON	LOW

#define LCD_I2C_ADDRESS	0x27	//	usually 0x27 or 0x3f
#define LCD_WIDTH	16		//	in characters
#define LCD_HEIGHT	2		//	in lines

#define	ENFORCE_POS	false	//	enforce position requirements
#define	ENFORCE_DT	true	//	enforce date-time requirements

#define GATE_OPEN_PULSE_WIDTH	1000

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
