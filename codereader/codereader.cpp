// Do not remove the include below
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "codereader.h"
#include "config.h"
#include "decode433.h"
#include "SdFat.h"
#include "sdfatlogwriter.h"
#include "ds3231.h"

SdFat			g_sd;
sdfatlogwriter	g_logger( g_sd );

LiquidCrystal_I2C	g_lcd( LCD_I2C_ADDRESS, LCD_WIDTH, LCD_HEIGHT );

//The setup function is called once at startup of the sketch

ts		g_dt;

void setup()
{
	Serial.begin( BAUDRATE );
	delay(10);
	for( char c = 0; c < 79; ++c ) Serial.print('-');
	Serial.println();

	setup433();
	g_codeready = false;
	g_code = 0;

	g_lcd.init();
	g_lcd.backlight();

	if( g_sd.begin( SS, SPI_HALF_SPEED )) {
		if( !g_logger.init() )
			Serial.println(F("Logger fail"));
	} else
		Serial.println(F("SD fail"));

	g_logger.log( logwriter::DEBUG, g_dt, F("Start"));
}

// The loop function is called in an endless loop
void loop()
{
	static uint16_t	code(0);
	static uint8_t	cnt(0);

	if( g_codeready )
	{
		if( code != g_code ) {
			if( cnt ) {
				Serial.print( F("Aborting ") );
				Serial.print( code );
				Serial.print( ' ' );
				Serial.println( g_code );
			}
			code = g_code;
			cnt = 0;
		} else if( cnt++ > 1 ) {
			uint16_t	id( getid( g_code ));
			Serial.println( id );
			g_lcd.setCursor(0,0);
			if(id < 10 ) g_lcd.print(' ');
			if(id < 100 ) g_lcd.print(' ');
			if(id < 1000 ) g_lcd.print(' ');
			g_lcd.print( id );
			g_lcd.print('.');
			g_lcd.print( getbutton( g_code ) );
			g_lcd.print(F("   "));
			g_logger.log( logwriter::INFO, g_dt, F("Remote"), id, getbutton( g_code ));
			cnt = 0;
		} else Serial.print('.');
		g_codeready = false;
	}
}
