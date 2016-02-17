/*
 * config.h
 *
 *  Created on: Feb 6, 2016
 *      Author: compi
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#define BAUDRATE 115200
#define DECODE433_REVERSE
#define USE_I2CDB
#define I2CDB_EEPROM_ADDRESS	0x50
#define I2CDB_EEPROM_BITS		16
#define	I2CDB_EEPROMPAGE_LENGTH	128
#define I2CDB_EEPROM_OFFSET		0

#define PIN_RFIN		2
#define PIN_GATE		A2

#define RELAY_OFF	HIGH
#define RELAY_ON	LOW

#define OUT_GREEN	6
#define OUT_YELLOW	5
#define	OUT_RED		4
#define IN_GREEN	9
#define IN_YELLOW	8
#define IN_RED		7

#define ID_MIN	4
#define	ID_MAX	1019

#endif /* CONFIG_H_ */
