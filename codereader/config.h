/*
 * config.h
 *
 *  Created on: Feb 7, 2016
 *      Author: compi
 */

#ifndef CONFIG_H_
#define CONFIG_H_

//#define FAILSTATS
//#define USE_SDCARD
//#define USE_RTC
#define USE_LCD


#define BAUD 115200

//#define DECODE433_REVERSE

#define PIN_RFIN		2

#ifdef USE_LCD
#define LCD_I2C_ADDRESS	0x27	//	usually 0x27 or 0x3f
#define LCD_WIDTH	16		//	in characters
#define LCD_HEIGHT	2		//	in lines
#endif	//	USE_LCD

#endif /* CONFIG_H_ */
