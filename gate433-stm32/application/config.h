/*
 * config.h
 *
 *  Created on: Jan 23, 2018
 *      Author: compi
 */
#ifndef CONFIG_H_
#define CONFIG_H_
#include <sg/Singleton.h>

#define CFG_I2C_ADDRESS		(0x57 << 1)
#define CFG_ADDRESS_LENGTH	2
#define CFG_PAGE_LENGTH		32

struct Config : public sg::Singleton<Config>
{
	friend class sg::Singleton<Config>;
	uint8_t 	magic, version;
	uint8_t		dbI2cAddress = 0x50 << 1;
	uint8_t 	dbAddresLength = 2;
	uint8_t		dbPageLength = 128;
	uint8_t 	lcdI2cAddress = 0x27 << 1;
	uint8_t		passTimeout = 30;
	uint8_t		hurryTimeout = 90;
	bool		relaxedPos = false;
	bool		relaxedDateTime = false;

};

#endif /* CONFIG_H_ */
