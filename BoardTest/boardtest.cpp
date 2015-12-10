// Do not remove the include below
#include <Wire.h>
#include <ds3231.h>
#include <LiquidCrystal_I2C.h>
#include "interface.h"
#include "boardtest.h"

SdFat	g_sd;

File 	g_info;
File 	g_status;
char	g_buffer[256+1];
int		g_nread(0), g_ntotal(0);
uint8_t	pins[8] = { 9,8,7,6,5,4,A3,A2 };

LiquidCrystal_I2C g_lcd(0x3f,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

inline void lcdout() {}
template< typename Arg1, typename... Args> void lcdout( const Arg1& arg1, const Args&... args)
{
	g_lcd.print( arg1 );
	lcdout( args...);
}

void printstatus( uint8_t pin )
{
	static ts	prevt = {0,0,0,0,0,0,0,0,0,0};
	ts	t;
	char	lcdbuffer[17];
	char	*lbp(lcdbuffer);

	DS3231_get( &t );

	if( prevt.year != t.year || prevt.mon != t. mon || prevt.mday != t.mday ) {
		g_lcd.setCursor(0,0);
		datetostring( lbp, t.year, t.mon, t.mday, t.wday, '.', '/' ); *lbp = 0;
		g_lcd.print( lcdbuffer );
	}

	g_lcd.setCursor(0,1);
	lbp = lcdbuffer;
	timetostring( lbp, t.hour, t.min, t.sec, ':' ); *lbp++ = 0;
	g_lcd.print( lcdbuffer);

	lbp = lcdbuffer;
	uitodec( pin, lbp, 2 ); *lbp++ = 0;
	g_lcd.setCursor(14,1);
	g_lcd.print( lcdbuffer );

	serialout( (uint16_t)t.year, F("."));
	serialout( (uint16_t)t.mon,F("."));
	serialout( (uint16_t)t.mday, F("/" ),(uint16_t)t.wday);
	serialout( F("    "), (uint16_t)t.hour);
	serialout(F(":" ), (uint16_t)(t.min));
	serialout(F(":" ), (uint16_t)(t.sec));
	serialout(F("    "), pin );
	Serial.println();
}

//The setup function is called once at startup of the sketch
void setup()
{
	bool	sdpass(true);

	g_lcd.init();                      // initialize the lcd
	g_lcd.backlight();
	Serial.begin( 115200 );
	for( int pin=0; pin<sizeof(pins); ++pin ) {
		pinMode( pins[pin], OUTPUT);
		digitalWrite( pins[pin], HIGH );
	}
	Wire.begin();
	DS3231_init( DS3231_INTCN );

	sdpass = g_sd.begin( SS, SPI_HALF_SPEED );
	serialout( F("SD card initialization "), sdpass ? F("succeeded.") : F("failed.") );
	Serial.println();
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
			sdpass = false;
		}

		g_info.close();
	} else {
		Serial.println( F("Opening info.txt failed."));
		sdpass = false;
	}

	lcdout( F("SD "), sdpass ? F("OK") : F("FAIL"));
	delay(5000);

	printstatus( 0 );
}

// The loop function is called in an endless loop
void loop()
{
//Add your repeated code here
	for( int pin=0; pin<sizeof(pins); ++pin)
	{
		printstatus(pins[pin]);
		delay(5);
		digitalWrite( pins[pin], LOW );
		delay(995);
		digitalWrite( pins[pin], HIGH );
	}
}
