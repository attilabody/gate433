// Do not remove the include below
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "codereader.h"
#include "config.h"
#include "decode433.h"
#include "ds3231.h"
#ifdef	USE_SDCARD
#include "SdFat.h"
#include "sdfatlogwriter.h"
SdFat			g_sd;
sdfatlogwriter	g_logger( g_sd );
#else
#include "dummylogwriter.h"
dummylogwriter	g_logger;
#endif	//	USE_SDCARD

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

#ifdef	USE_SDCARD
	if( g_sd.begin( SS, SPI_HALF_SPEED )) {
		if( !g_logger.init() )
			Serial.println(F("Logger fail"));
	} else
		Serial.println(F("SD fail"));
#endif	//	USE_SDCARD

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
			uint8_t	btn( getbutton( g_code ));
			Serial.print( id );
			Serial.print(F(", "));
			Serial.println((uint16_t) btn );
			g_lcd.setCursor(0,0);
			if(id < 10 ) g_lcd.print(' ');
			if(id < 100 ) g_lcd.print(' ');
			if(id < 1000 ) g_lcd.print(' ');
			g_lcd.print( id );
			g_lcd.print('.');
			g_lcd.print( getbutton( g_code ) );
			g_logger.log( logwriter::INFO, g_dt, F("Remote"), id, getbutton( g_code ));
			cnt = 0;
		} else Serial.print('.');
		g_codeready = false;
	}
}
