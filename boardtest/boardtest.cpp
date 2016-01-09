// Do not remove the include below
#include <Arduino.h>
#include <SdFat.h>
#include <Wire.h>
#include <ds3231.h>
#include <LiquidCrystal_I2C.h>
#include <PCF8574.h>
#include <I2C_eeprom.h>
#include "config.h"
#include "interface.h"
#include "intdb.h"
#include "thindb.h"
#include "hybriddb.h"
#include "flashdb.h"
#include "decode433.h"

#define TEST_SDCARD
#define TEST_LCD
#define TEST_DS3231

char		g_inbuf[256+1];
uint8_t	g_inidx(0);

#ifdef TEST_SDCARD
SdFat		g_sd;
flashdb		g_db( 0x50, 16, 128 );
//#else
#endif	//	TEST_SDCARD

uint8_t			g_pins[8] = { 0, 1, 2, 3, 7, 6, 5, 4 };
uint8_t			g_pinindex(0xff);
unsigned long	g_rtstart(0);
uint16_t		g_rtdelay;
PCF8574			g_i2cio(0x20);

#ifdef	TEST_LCD
#ifndef LCD_I2C_ADDRESS
#define LCD_I2C_ADDRESS 0x27
#endif	//	LCD_I2C_ADDRESS
//LiquidCrystal_I2C g_lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C g_lcd(LCD_I2C_ADDRESS,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

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
	g_i2cio.write8( 0xff );
//	for( uint8_t pin = 0; pin < sizeof(g_pins); ++ pin )
//		g_i2cio.write( pin, RELAY_OFF );
}

//////////////////////////////////////////////////////////////////////////////
void processInput()
{
	ts			t;
	const char	*inptr( g_inbuf );
//	char 		command( findcommand( inptr, g_commands ));

	relaysoff();

	if( iscommand( inptr, F("sdt"))) {
#ifdef	TEST_DS3231
		if( parsedatetime( t, inptr )) {
			DS3231_set( t );
			Serial.println( F(RESPS "OK"));
		} else
			Serial.println( F(ERRS ERRS " (DATETIMEFMT)"));
#else	//	TEST_DS3231
		Serial.println( F(ERRS "NOTIMPL"))
#endif	//	TEST_DS3231

	} else if( iscommand( inptr, F("rt"))) {
		long	delay( getintparam( inptr ));
		g_rtdelay = delay > 0 ? delay : 1000;
		g_pinindex = sizeof( g_pins ) - 1;
		g_rtstart = millis();

	} else if( iscommand( inptr, F("de"))) { 	//	dump eeprom
		uint8_t		b;
		uint16_t	address = getintparam( inptr );
		uint16_t	count = getintparam( inptr );
		for( int addr = 0; addr < count; ++addr )
		{
			if( addr && !( addr & 0xf ))	Serial.println();
			else if( addr ) Serial.print(' ');
			b = g_db.read_byte( address + addr );
			Serial.print( halfbytetohex( b >> 4));
			Serial.print( halfbytetohex( b & 0xf));
			delay(10);
		}

	} else if( iscommand( inptr, F("dep"))) {	//dump eeprom paged
		uint8_t		buffer[16];
		uint16_t	address = getintparam( inptr );
		uint16_t	count = getintparam( inptr );
		if(address == 0xffff) address = 0;
		address &= 0xfff0;

		for( uint32_t pageoffset = 0; pageoffset < count; pageoffset += 16 )
		{
			Serial.println();
			char	*bp((char*)buffer);
			uitohex( bp, pageoffset, 4 );
			*bp++ = ' ';
			*bp = 0;
			Serial.print((char*) buffer );

			g_db.read_page( address + pageoffset, buffer, 16 );
			for( uint8_t offset=0; offset < 16; ++offset ) {
				if( offset) Serial.print(' ');
				Serial.print( halfbytetohex( buffer[offset] >> 4));
				Serial.print( halfbytetohex( buffer[offset] & 0xf));
			}
		}

	} else if( iscommand( inptr, F("se"))) {	//	set eeprom
		uint16_t	address = getintparam( inptr );
		uint16_t	value = getintparam( inptr );
		g_db.write_byte( address, value );

	} else if( iscommand( inptr, F("ge"))) {	//	get eeprom
		uint16_t	address = getintparam( inptr );
		uint8_t		value = g_db.read_byte( address );
		Serial.print( halfbytetohex( value >> 4 ));
		Serial.println( halfbytetohex( value & 0xf ));

	} else if( iscommand( inptr, F("fe"))) {	//	fill eeprom <start> <value> <count>
		uint16_t	address = getintparam( inptr );
		uint8_t		value = getintparam( inptr );
		uint16_t	count = getintparam( inptr );
		g_db.fill_page( address, value, count );

	} else if( iscommand( inptr, F("ddb"))) {	//	dump database
		char recbuf[ INFORECORD_WIDTH + STATUSRECORD_WIDTH + 1 ];
		database::dbrecord	rec;
		uint16_t	start( getintparam( inptr ));
		uint16_t	count( getintparam( inptr ));
		if( start == 0xffff ) start = 0;
		if( count == 0xffff ) count = 1024 - start;
		for( uint16_t code = start; code < start + count; ++code ) {
			if( g_db.getParams( code, rec )) {
				rec.serialize( recbuf );
				Serial.println( recbuf );
			} else {
				Serial.println( F(ERRS "GETPARAMS" ));
			}
		}

	} else if( iscommand( inptr, F("fdb"))) {	//	fill database
		database::dbrecord	rec = { 0, 0xfff, 0, 0xfff, 0x7f, database::dbrecord::unknown };

		for( int code = 0; code < 1024; ++code ) {
			rec.in_start = code;
			g_db.setParams( code, rec );
		}

	} else if( iscommand( inptr, F("ftdb"))) {	//	fill thin database
		database::dbrecord	rec = { 0, 0xfff, 0, 0xfff, 0x7f, database::dbrecord::unknown };
		thindb				tdb( g_sd );
		if( tdb.init()) {
			for( int code = 0; code < 1024; ++code ) {
				rec.in_start = code;
				tdb.setParams( code, rec );
			}
		}

	} else if( iscommand( inptr, F("cls"))) {
		g_db.cleanstatuses();

	} else if( iscommand( inptr, F("gett"))) {	//	get thindb
		database::dbrecord	rec;
		char 				recbuf[ DBRECORD_WIDTH + 1 ];
		int 				code( getintparam( inptr ));
		thindb				tdb( g_sd );
		if( tdb.init() && code != -1 ) {
			tdb.getParams( code, rec );
			rec.serialize( recbuf );
			Serial.println( recbuf );
		}

	} else if( iscommand( inptr, F("get"))) {
		database::dbrecord	rec;
		char 				recbuf[ DBRECORD_WIDTH + 1 ];
		int 				code( getintparam( inptr ));
		if( code != -1 ) {
			g_db.getParams( code, rec );
			rec.serialize( recbuf );
			Serial.println( recbuf );
		}

	} else if( iscommand( inptr, F("set"))) {
		database::dbrecord	rec;
		int 				code( getintparam( inptr ));
		if( code != -1 && rec.parse( inptr )) {
			g_db.setParams( code, rec );
		}

	} else if( iscommand( inptr, F("sett"))) {	//	set thindb
		database::dbrecord	rec;
		thindb	tdb( g_sd );
		if( tdb.init())
		{
			int code( getintparam( inptr ));
			if( code != -1 && rec.parse( inptr )) {
				tdb.setParams( code, rec );
			}
		}

	} else if( iscommand( inptr, F("it"))) {	//import thindb
		thindb		tdb( g_sd );
		uint16_t	from( getintparam( inptr ));
		uint16_t	to( getintparam( inptr ));
		if( from == 0xffff ) from = 0;
		if( to == 0xffff ) to = 1023;
		if( tdb.init())
		{
			database::dbrecord	rec;
			for( uint16_t id = from; id <= to; ++id ) {
				if( tdb.getParams( id, rec ))
					g_db.setParams( id, rec );
			}
		}
	} else if( iscommand( inptr, F("echo"))) {
		long param;
		while((param = getintparam( inptr, true, true, true )) != LONG_MIN ) {
			Serial.print( param );
			Serial.print(' ');
		}
		Serial.println();

	} else if( iscommand( inptr, F("echo"))) {
		long param;
		while((param = getintparam( inptr, true, true, true )) != LONG_MIN ) {
			Serial.print( param );
			Serial.print(' ');
		}
		Serial.println();

	} else {
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
	Serial.println();
	serialoutln(F( CMNTS "------------------ Setup ------------------"));
	delay(100);

	g_lcd.init();                      // initialize the lcd
	g_lcd.backlight();

	for( size_t pin=0; pin<sizeof(g_pins); ++pin ) {
		pinMode( g_pins[pin], OUTPUT);
		g_i2cio.write( g_pins[pin], HIGH );
	}

	Wire.begin();
	DS3231_init( DS3231_INTCN );

	g_sd.begin( SS );
	g_db.init();

	attachInterrupt( digitalPinToInterrupt( PIN_RFIN ), isr, CHANGE );
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
		if( millis() - g_rtstart > g_rtdelay )
		{
			uint8_t	prevpin = g_pinindex++;
			if( g_pinindex >= sizeof( g_pins ))
				g_pinindex = 0;
			g_i2cio.write( g_pins[prevpin], RELAY_OFF );
			g_i2cio.write( g_pins[g_pinindex], RELAY_ON );
			g_rtstart += g_rtdelay;
			printpin( g_pins[g_pinindex] );
		}
	}

	delay(10);
}
