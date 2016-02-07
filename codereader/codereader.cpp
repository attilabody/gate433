// Do not remove the include below
#include <LiquidCrystal_I2C.h>
#include "codereader.h"
#include "config.h"
#include "decode433.h"

LiquidCrystal_I2C	g_lcd( LCD_I2C_ADDRESS, LCD_WIDTH, LCD_HEIGHT );

//The setup function is called once at startup of the sketch
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

}

// The loop function is called in an endless loop
void loop()
{
	static uint16_t	code(0);
	static uint8_t	cnt(0);

	if( g_codeready )
	{
		if( code != g_code ) {
			Serial.print( F("Aborting ") );
			Serial.print( code );
			Serial.print( ' ' );
			Serial.println( g_code );
			code = g_code;
			cnt = 0;
		} else if( cnt++ > 3 ) {
			Serial.println( getid(g_code) );
			g_lcd.setCursor(0,0);
			g_lcd.print( getid( g_code ), 1);
			g_lcd.print(' ');
			g_lcd.print( getbutton( g_code ) );
			g_lcd.print(F("   "));
			cnt = 0;
		}
		g_codeready = false;
	}
}
