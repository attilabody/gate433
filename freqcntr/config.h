/*
 * config.h
 *
 *  Created on: Apr 20, 2016
 *      Author: abody
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#define BAUDRATE 115200

#define USE_I2CLCD

#ifdef USE_I2CLCD
#define LCD_I2C_ADDRESS	0x27	//	usually 0x27 or 0x3f
#define LCD_WIDTH	16		//	in characters
#define LCD_HEIGHT	2		//	in lines
#endif	//	USE_I2CLCD

#endif /* CONFIG_H_ */
