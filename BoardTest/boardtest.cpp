// Do not remove the include below
#include <Wire.h>
#include <ds3231.h>

#include "boardtest.h"

SdFat	g_sd;
bool	g_sdInit(false);

File 	g_info;
File 	g_status;
char	g_buffer[256+1];
int		g_nread(0), g_ntotal(0);
uint8_t	pins[8] = { 9,8,7,6,5,4,A3,A2 };

inline void serialout() { Serial.println(); }
template< typename Arg1, typename... Args> void serialout( const Arg1& arg1, const Args&... args)
{
	Serial.print( arg1 );
	serialout( args...);
}

//The setup function is called once at startup of the sketch
void setup()
{
// Add your initialization code here
	Serial.begin( 115200 );
	for( int pin=0; pin<sizeof(pins); ++pin ) {
		pinMode( pins[pin], OUTPUT);
		digitalWrite( pins[pin], HIGH );
	}
	Wire.begin();
	DS3231_init( DS3231_INTCN );

	g_sdInit = g_sd.begin( SS, SPI_HALF_SPEED );
	serialout( F("SD card initialization "), g_sdInit ? F("succeeded.") : F("failed.") );

	g_info = g_sd.open("info.txt", FILE_READ );
	if( g_info )
	{

		g_buffer[sizeof(g_buffer)-1] = 0;

		while( (g_nread = g_info.read(g_buffer, sizeof(g_buffer)-1))) {
			g_ntotal += g_nread;
		}
		Serial.print( g_ntotal );
		Serial.println(F(" bytes read"));

		if(!g_info.seek( 0 )) {
			Serial.println(F("seek failed"));
		}

		g_info.close();
	} else
		Serial.println( F("Opening info.txt failed."));

	ts	t;
	DS3231_get( &t );
	Serial.print( (uint16_t)t.year); Serial.print("." );
	Serial.print( (uint16_t)t.mon ); Serial.print("." );
	Serial.print( (uint16_t)t.mday ); Serial.print("/" ); Serial.print((uint16_t)t.wday);
	Serial.print("    "); Serial.print( (uint16_t)t.hour);
	Serial.print(":" ); Serial.print( (uint16_t)(t.min));
	Serial.print(":" ); Serial.print( (uint16_t)(t.sec));
	Serial.println();

}

// The loop function is called in an endless loop
void loop()
{
//Add your repeated code here
	for( int pin=0; pin<sizeof(pins); ++pin)
	{
		Serial.println(pins[pin]);
		delay(5);
		digitalWrite( pins[pin], LOW );
		delay(995);
		digitalWrite( pins[pin], HIGH );
	}
}
