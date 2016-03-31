/*
 * loggertest.cpp
 *
 *  Created on: Jan 27, 2016
 *      Author: abody
 */
#include <Arduino.h>
#include <I2C.h>
#include <ds3231.h>
#include <PCF8574.h>
#include <LiquidCrystal_I2C.h>
#include "config.h"
#include "globals.h"
#include "toolbox.h"
#include "commsyms.h"

ts					g_time;
PCF8574				g_ioext( PCF8574_ADDRESS );
uint8_t				g_portstatus( 0xa5 );
LiquidCrystal_I2C	g_lcd( LCD_I2C_ADDRESS, LCD_WIDTH, LCD_HEIGHT );

//////////////////////////////////////////////////////////////////////////////
void processinput();

//////////////////////////////////////////////////////////////////////////////
void setup()
{
	Serial.begin( 115200 );
	I2c.begin();
	I2c.timeOut(1000);
	DS3231_DST::init( DS3231_INTCN );
	DS3231_DST::get( &g_time );
	g_lcd.init();
	g_lcd.backlight();
	g_lcd.print("Minden fasza");
}

//////////////////////////////////////////////////////////////////////////////
void loop()
{
	static unsigned long	lasttick(0);
	unsigned long			now(millis());

	if( getlinefromserial( g_iobuf, sizeof( g_iobuf ), g_inidx) )
		processinput();
	if( now - lasttick > 1000 ) {
		lasttick += 1000;
		g_ioext.write8(g_portstatus);
		g_portstatus ^= 0xff;
	}
}

//////////////////////////////////////////////////////////////////////////////
void processinput()
{
	const char	*inptr( g_iobuf );

	Serial.print(CMNT);
	Serial.println( g_iobuf );

	if( iscommand( inptr, F("xxx"))) {
//		doSomething();
	} else {
		Serial.println( F(ERRS "CMD"));
	}
	g_inidx = 0;
}
