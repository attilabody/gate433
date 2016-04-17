// Do not remove the include below
#include <Arduino.h>
#include "LiquidCrystal.h"
#include "decode433.h"

//////////////////////////////////////////////////////////////////////////////
uint8_t			g_ledpin(13);
LiquidCrystal	g_lcd(4,5,6,7,8,9);

//////////////////////////////////////////////////////////////////////////////
void setup()
{
  Serial.begin(19200);
  g_lcd.begin(16, 2);
  pinMode(g_ledpin, OUTPUT);
  setup433();
  noInterrupts();           // disable all interrupts
  TIMSK0 |= (1 << OCIE0A);  // enable timer compare interrupt
  interrupts();             // enable all interrupts

}

//////////////////////////////////////////////////////////////////////////////
void loop()
{
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

			g_lcd.setCursor(0,0);
			if(id < 10 ) g_lcd.print(' ');
			if(id < 100 ) g_lcd.print(' ');
			if(id < 1000 ) g_lcd.print(' ');
			g_lcd.print( id );
			g_lcd.print('.');
			g_lcd.print( getbutton( g_code ) );
			cnt = 0;
		} else Serial.print('.');
		g_codeready = false;
	}
}

//////////////////////////////////////////////////////////////////////////////
ISR( TIMER0_COMPA_vect )
{
  unsigned long now(micros());
  static bool	ledstatus(false);

  if( now - g_codetime < 500000 ) {
	  if(!ledstatus) {
		  digitalWrite(g_ledpin, HIGH);
		  ledstatus = true;
	  }
  } else {
	  if(ledstatus) {
		  digitalWrite(g_ledpin, LOW);
		  ledstatus = false;
	  }
  }
}


