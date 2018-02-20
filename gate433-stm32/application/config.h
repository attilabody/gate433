/*
 * config.h
 *
 *  Created on: Jan 23, 2018
 *      Author: compi
 */
#ifndef CONFIG_H_
#define CONFIG_H_
#include <sg/singleton.h>

#define DB_I2C_ADDRESS		(0x50 << 1)
#define DB_ADDRESS_LENGTH	2
#define DB_PAGE_LENGTH		128
#define CFG_I2C_ADDRESS		(0x57 << 1)
#define CFG_ADDRESS_LENGTH	2
#define CFG_PAGE_LENGTH		32

#define LCD_I2C_ADDRESS			(0x27 << 1)

struct Config : public sg::Singleton<Config>
{
	friend class sg::Singleton<Config>;
	uint8_t 	magic, version;
	uint8_t		dbI2cAddress = DB_I2C_ADDRESS;
	uint8_t 	dbAddresLength = DB_ADDRESS_LENGTH;
	uint8_t		dbPageLength = DB_PAGE_LENGTH;
	uint8_t 	lcdI2cAddress = LCD_I2C_ADDRESS;
	uint8_t 	lcdWidth = 16;
	uint8_t 	lcdHeight = 2;
	uint8_t		passTimeout = 30;
	uint8_t		hurryTimeout = 90;

};

#endif /* CONFIG_H_ */
