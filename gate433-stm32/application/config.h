/*
 * config.h
 *
 *  Created on: Jan 23, 2018
 *      Author: compi
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#define DB_I2C_ADDRESS		(0x50 << 1)
#define DB_ADDRESS_LENGTH	2
#define CFG_I2C_ADDRESS		(0x57 << 1)
#define CFG_ADDRESS_LENGTH	2

#define LCD_I2C_ADDRESS			(0x27 << 1)

struct Config
{
	uint8_t	dbI2cAddress = DB_I2C_ADDRESS;
	uint8_t lcdI2cAddress = LCD_I2C_ADDRESS;
};

#endif /* CONFIG_H_ */
