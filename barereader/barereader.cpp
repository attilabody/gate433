// Do not remove the include below
#include <Arduino.h>
#include "LiquidCrystal.h"
#include "decode433.h"

LiquidCrystal lcd(4,5,6,7,8,9);

void setup()
{
  Serial.begin(19200);
  lcd.begin(16, 2);
//  lcd.print("hello, world!");
  pinMode(13, OUTPUT);
  setup433();
}

void loop()
{
//  static uint16_t lastsec(0);
//  lcd.setCursor(0, 1);
//  uint16_t cursec(millis()/1000);
//  if(lastsec != cursec) {
//    lcd.print(cursec);
//    digitalWrite(13, (cursec & 1) ? HIGH : LOW);
//    Serial.println(cursec);
//    lastsec = cursec;
//  }
	static uint16_t	code(0);
	static uint8_t	cnt(0);

	if( g_codeready )
	{
		uint16_t	id( getid( g_code ));
		uint8_t	btn( getbutton( g_code ));

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
			Serial.print( id );
			Serial.print(F(", "));
			Serial.println((uint16_t) btn );

			lcd.setCursor(0,0);
			if(id < 10 ) lcd.print(' ');
			if(id < 100 ) lcd.print(' ');
			if(id < 1000 ) lcd.print(' ');
			lcd.print( id );
			lcd.print('.');
			lcd.print( getbutton( g_code ) );
			cnt = 0;
		} else Serial.print('.');
		g_codeready = false;
	}
}

