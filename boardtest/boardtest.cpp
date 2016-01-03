// Do not remove the include below
#include <Arduino.h>
#include <SdFat.h>
#include <Wire.h>
#include <ds3231.h>
#include <LiquidCrystal_I2C.h>
#include "config.h"
#include "interface.h"
#include "intdb.h"
#include "decode433.h"

#define TEST_SDCARD
#define TEST_LCD
#define TEST_DS3231

char		g_inbuf[256+1];
uint16_t	g_inidx(0);
const char 		*g_commands[] = {
	  "sdt"		//set datetime
	, "ddb"		//dump db
	, "rs"		//relay stop
	, "rt"		//relay test
	, "bt"		//blink test
	, ""
};

#ifdef TEST_SDCARD
SdFat		g_sd;
intdb		g_db( g_sd, false );
//#else
#endif	//	TEST_SDCARD

uint8_t			g_pins[8] = { 9,8,7,6,5,4,A3,A2 };
uint8_t			g_pinindex(0xff);
unsigned long	g_rtstart(0);

#ifdef	TEST_LCD
LiquidCrystal_I2C g_lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display
//LiquidCrystal_I2C g_lcd(0x3f,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

inline void lcdout() {}
template< typename Arg1, typename... Args> void lcdout( const Arg1& arg1, const Args&... args)
{
	g_lcd.print( arg1 );
	lcdout( args...);
}

#endif	//	TEST_LCD

//////////////////////////////////////////////////////////////////////////////
void printpin( uint8_t pin );

//////////////////////////////////////////////////////////////////////////////
void relaysoff()
{
	g_pinindex = 0xff;
	printpin( 0xff );
}

//////////////////////////////////////////////////////////////////////////////
void processInput()
{
	ts			t;
	const char	*inptr( g_inbuf );
	char 		command( findcommand( inptr, g_commands ));

	relaysoff();

	serialoutln( CMNT, (uint16_t)command );

	switch( command ) {
	case 0:		//	sdt
#ifdef	TEST_DS3231
		if( parsedatetime( t, inptr )) {
			DS3231_set( t );
			Serial.println( F(RESPS "OK"));
		} else
			Serial.println( F(ERRS ERRS " (DATETIMEFMT)"));
#else	//	TEST_DS3231
		Serial.println( F(ERRS "NOTIMPL"))
#endif	//	TEST_DS3231
		break;

	case 1:		//	ddb
#ifdef TEST_SDCARD
		{
			char recbuf[ INFORECORD_WIDTH + STATUSRECORD_WIDTH + 1 ];
			database::dbrecord	rec;

			for( int code = 0; code < 1024; ++code ) {
				if( g_db.getParams( code, rec )) {
					rec.serialize( recbuf );
					Serial.println( recbuf );
				} else {
					Serial.println( F(ERRS "GETPARAMS" ));
				}

			}
		}
#else	//	TEST_SDCARD
		Serial.println( F(ERRS "NOTIMPL"));
#endif	//	TEST_SDCARD
			break;

	case 2:		// relay stop
		break;

	case 3:		// relay test
		g_pinindex = sizeof( g_pins ) - 1;
		break;

	default:
		Serial.println( ERRS "CMD");

	}
	g_inidx = 0;
}

//////////////////////////////////////////////////////////////////////////////
void printdatetime()
{
#ifdef	TEST_DS3231
	static ts		prevt = {0,0,0,0,0,0,0,0,0,0};

	ts	t;
	char	lcdbuffer[17];
	char	*lbp(lcdbuffer);

	DS3231_get( &t );

#ifdef	TEST_LCD
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

#else	//	TEST_LCD

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
#endif	//	TEST_LCD

	prevt = t;
#endif	//	TEST_DS3231
}

//////////////////////////////////////////////////////////////////////////////
void printpin( uint8_t pin )
{
#ifdef TEST_LCD
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
		g_lcd.setCursor(10,1);
		g_lcd.print( lcdbuffer );
		prevpin = pin;
	}
#endif	//	TEST_LCD
}

//////////////////////////////////////////////////////////////////////////////
void setup()
{
	Serial.begin( 115200 );
	delay(100);
	serialout(CMNTS "Setup\n");
	delay(100);

#ifdef	TEST_LCD
	g_lcd.init();                      // initialize the lcd
	g_lcd.backlight();
#endif	//	TEST_LCD

	for( size_t pin=0; pin<sizeof(g_pins); ++pin ) {
		pinMode( g_pins[pin], OUTPUT);
		digitalWrite( g_pins[pin], HIGH );
	}

#ifdef TEST_DS3231
#ifndef TEST_LCD
	Wire.begin();
#endif	//	TEST_LCD
	DS3231_init( DS3231_INTCN );
#endif	//	TEST_DS3231

#ifdef TEST_SDCARD
	bool sdinit( g_sd.begin( SS ));
	bool dbinit( g_db.init() );
#ifdef TEST_LCD
	lcdout( F("DB "), dbinit ? F("OK") : F("FAIL"));
#endif	//	TEST_LCD
	serialout( CMNTS "DB ", dbinit ? F("OK") : F("FAIL"));
	delay(3000);
#endif	//	TEST_SDCARD
	attachInterrupt( digitalPinToInterrupt( PIN_RFIN ), isr, CHANGE );
	printdatetime();
}

//////////////////////////////////////////////////////////////////////////////
void loop()
{
	if( getlinefromserial( g_inbuf, sizeof(g_inbuf), g_inidx )) {
		processInput();
	}

	unsigned long curmillis( millis() );

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
			g_rtstart = curmillis;
			printpin( g_pins[g_pinindex] );
		}
	}

	delay(10);
}
