// Do not remove the include below
#include <Wire.h>
#include <ds3231.h>
#include <LiquidCrystal_I2C.h>
#include "interface.h"
#include "intdb.h"
#include "boardtest.h"

//char 		g_inbuf[256];
char		g_inbuf[256+1];
uint16_t	g_inidx(0);
const char 		*g_commands[] = {
	  "sdt"		//set datetime
	, "ddb"		//dump db
	, "rt"		//relay test
	, "rs"		//relay stop
	, ""
};

intdb		g_db;

uint8_t			g_pins[8] = { 9,8,7,6,5,4,A3,A2 };
uint8_t			g_pinindex(0xff);
unsigned long	g_rtstart(0);

LiquidCrystal_I2C g_lcd(0x3f,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

//////////////////////////////////////////////////////////////////////////////
void printpin( uint8_t pin );

//////////////////////////////////////////////////////////////////////////////
inline void lcdout() {}
template< typename Arg1, typename... Args> void lcdout( const Arg1& arg1, const Args&... args)
{
	g_lcd.print( arg1 );
	lcdout( args...);
}

//////////////////////////////////////////////////////////////////////////////
void relaysoff()
{
	for( uint8_t pin = 0; pin < sizeof( g_pins ); ++pin )
		digitalWrite( g_pins[pin], HIGH );
	g_pinindex = 0xff;
	printpin( 0xff );
}

//////////////////////////////////////////////////////////////////////////////
void processInput()
{
	ts			t;
	const char	*inptr( g_inbuf );
	char 		command( findcommand( inptr, g_commands ));
	switch( command ) {
	case 0:		//	sdt
		relaysoff();

		if( parsedatetime( t, inptr )) {
			DS3231_set( t );
			Serial.println( F(RESPS "OK"));
		} else
			Serial.println( F(ERRS ERRS " (DATETIMEFMT)"));
		break;

	case 1:		//	ddb
		{
			char recbuf[ INFORECORD_WIDTH + STATUSRECORD_WIDTH + 1 ];

			relaysoff();

			for( int code = 0; code < 1024; ++code )
			{
				database::dbrecord	rec;
				if( g_db.getParams( code, rec ))
				{
					rec.serialize( recbuf );
					Serial.println( recbuf );
				} else {
					Serial.println( F(ERRS " (GETPARAMS)" ));
				}

			}
			break;
		}
	case 2:		//	relay
		g_pinindex = sizeof( g_pins ) - 1;
		break;
	case 3:
		relaysoff();
		break;
	}
	g_inidx = 0;
}

//////////////////////////////////////////////////////////////////////////////
void printdatetime()
{
	static ts		prevt = {0,0,0,0,0,0,0,0,0,0};

	ts	t;
	char	lcdbuffer[17];
	char	*lbp(lcdbuffer);

	DS3231_get( &t );

	if( prevt.year != t.year || prevt.mon != t. mon || prevt.mday != t.mday ) {
		g_lcd.setCursor(0,0);
		datetostring( lbp, t.year, t.mon, t.mday, t.wday, '.', '/' ); *lbp = 0;
		g_lcd.print( lcdbuffer );
	}

	if( prevt.hour != t.hour || prevt.min != t.min || prevt.sec != t.sec ) {
		g_lcd.setCursor(0,1);
		lbp = lcdbuffer;
		timetostring( lbp, t.hour, t.min, t.sec, ':' ); *lbp++ = 0;
		g_lcd.print( lcdbuffer);
	}

	if( prevt.year != t.year || prevt.mon != t. mon || prevt.mday != t.mday ||
		prevt.hour != t.hour || prevt.min != t.min || prevt.sec != t.sec )
	{
		serialout( (uint16_t)t.year, F("."));
		serialout( (uint16_t)t.mon,F("."));
		serialout( (uint16_t)t.mday, F("/" ),(uint16_t)t.wday);
		serialout( F("    "), (uint16_t)t.hour);
		serialout(F(":" ), (uint16_t)(t.min));
		serialout(F(":" ), (uint16_t)(t.sec));
		Serial.println();
	}

	prevt = t;
}

//////////////////////////////////////////////////////////////////////////////
void printpin( uint8_t pin )
{
	static uint8_t	prevpin( 0xff );
	char	lcdbuffer[3];
	char	*lbp(lcdbuffer);

	if( prevpin != pin )
	{
		if( pin != 0xff ) {
			lbp = lcdbuffer;
			uitodec( lbp, pin, 2 ); *lbp++ = 0;
		} else {
			lcdbuffer[0] = ' '; lcdbuffer[1] = ' '; lcdbuffer[2] = 0;
		}
		g_lcd.setCursor(14,1);
		g_lcd.print( lcdbuffer );
		prevpin = pin;
	}
}

//////////////////////////////////////////////////////////////////////////////
void setup()
{

	g_lcd.init();                      // initialize the lcd
	g_lcd.backlight();
	Serial.begin( 115200 );
	for( size_t pin=0; pin<sizeof(g_pins); ++pin ) {
		pinMode( g_pins[pin], OUTPUT);
		digitalWrite( g_pins[pin], HIGH );
	}
	Wire.begin();
	DS3231_init( DS3231_INTCN );

	lcdout( F("DB "), g_db.isinitsucceeded() ? F("OK") : F("FAIL"));
	delay(3000);

	printdatetime();
}

//////////////////////////////////////////////////////////////////////////////
void loop()
{
	if( getlinefromserial( g_inbuf, sizeof(g_inbuf), g_inidx )) {
		processInput();
	}

	printdatetime();

	if( g_pinindex != 0xff )
	{
		if( millis() - g_rtstart > 1000 )
		{
			uint8_t	prevpin = g_pinindex++;
			if( g_pinindex >= sizeof( g_pins ))
				g_pinindex = 0;
			digitalWrite( g_pins[prevpin], HIGH );
			digitalWrite( g_pins[g_pinindex], LOW );
			g_rtstart = millis();
			printpin( g_pins[g_pinindex] );
		}
	}
}
