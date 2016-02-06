// Do not remove the include below
#include "gk_simple.h"
#include "decode433.h"
#include "toolbox.h"
#include "commsyms.h"

//The setup function is called once at startup of the sketch
void setup()
{
	Serial.begin( BAUDRATE );
	delay(10);
	Serial.print( CMNT );
	for( char c = 0; c < 79; ++c ) Serial.print('-');
	Serial.println();

	pinMode( PIN_GATE, OUTPUT );
	digitalWrite( PIN_GATE, RELAY_OFF );
	setup433();
	g_codeready = false;
	g_code = 0;

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
			uint16_t	id( getid( g_code ));

			Serial.print( id );
			if( id >=4 )
			{
				Serial.println(F(" -> opening gate."));
				digitalWrite( PIN_GATE, RELAY_ON );
				delay(1000);
				digitalWrite( PIN_GATE, RELAY_OFF );
			} else {
				Serial.println(F(" -> ignoring."));
			}
			cnt = 0;
		}
		g_codeready = false;
	}
}
