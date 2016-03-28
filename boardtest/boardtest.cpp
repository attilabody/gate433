// Do not remove the include below
#include <Arduino.h>
#include <SdFat.h>
#include <ds3231.h>
#include <LiquidCrystal_I2C.h>
#include <PCF8574.h>
#include <I2C_eeprom.h>
#include <MemoryFree.h>
#include "config.h"
#include "commsyms.h"
#include "serialout.h"
#include "toolbox.h"
#include "dthelpers.h"
#include "intdb.h"
#include "thindb.h"
#include "hybriddb.h"
#include "hybintdb.h"
#include "i2cdb.h"
#include "eepromdb.h"
#include "decode433.h"
#include "sdfatlogwriter.h"
#include "arduinooutputs.h"

#define TEST_SDCARD
#define TEST_LCD
#define TEST_DS3231

char		g_inbuf[128+1];
uint8_t		g_inidx(0);

#ifdef TEST_SDCARD
SdFat			g_sd;
i2cdb			g_db( I2CDB_EEPROM_ADDRESS, I2CDB_EEPROM_BITS, 128 );
//eepromdb		g_db;
sdfatlogwriter	g_logger( g_sd );
uint16_t		g_lastcheckpoint(0);
#endif	//	TEST_SDCARD

uint8_t			g_pinindex( 8 - 1 );
uint16_t		g_rtdelay(1000);
unsigned long	g_rtstart(0);
ts				g_dt;

uint8_t			g_pinmap[] = {0,1,2,3,7,6,5,4};

PCF8574			g_ioext(PCF8574_ADDRESS);
const uint8_t	g_alloutputpins[8] = { ALL_RAW_OUTPUT_PINS };
arduinooutputs	g_outputs;

#ifdef	TEST_LCD
#ifndef LCD_I2C_ADDRESS
#define LCD_I2C_ADDRESS 0x27
#endif	//	LCD_I2C_ADDRESS
//LiquidCrystal_I2C g_lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C g_lcd(LCD_I2C_ADDRESS,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

template< typename Arg> void lcdout( const Arg arg ) { g_lcd.print( arg ); }
template< typename Arg1, typename... Args> void lcdout( const Arg1& arg1, const Args&... args)
{
	g_lcd.print( arg1 );
	lcdout( args...);
}

#endif	//	TEST_LCD

#ifdef TEST_DS3231
DS3231	g_clk;
#endif	//	TEST_DS3231

//////////////////////////////////////////////////////////////////////////////
void printpins( uint8_t status );
void printpin( uint8_t pin );

//////////////////////////////////////////////////////////////////////////////
void relaysoff()
{
	g_pinindex = 0xff;
	printpins( 0 );
	g_outputs.write8( 0xff );
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
	Serial.println( inptr );

	if( iscommand( inptr, F("sdt"))) {
#ifdef	TEST_DS3231
		if( parsedatetime( t, inptr )) {
			g_clk.set( t );
			Serial.println( F(RESPS "OK"));
		} else
			Serial.println( F(ERRS ERRS " (DATETIMEFMT)"));
#else	//	TEST_DS3231
		Serial.println( F(ERRS "NOTIMPL"))
#endif	//	TEST_DS3231

	} else if( iscommand( inptr, F("gdt"))) {	// get datetime
#ifdef	TEST_DS3231
		g_clk.get( &t );
		serialout( (uint16_t)t.year, F("."));
		serialout( (uint16_t)t.mon,F("."));
		serialout( (uint16_t)t.mday, F("/" ),(uint16_t)t.wday);
		serialout( F("    "), (uint16_t)t.hour);
		serialout(F(":" ), (uint16_t)(t.min));
		serialoutln(F(":" ), (uint16_t)(t.sec));
#else	//	TEST_DS3231
		Serial.println( F(ERRS "NOTIMPL"))
#endif	//	TEST_DS3231

	} else if( iscommand( inptr, F("rt"))) {
		long	delay( getintparam( inptr ));
		g_rtdelay = delay > 0 ? delay : 1000;
		g_pinindex = 8 - 1;
		g_rtstart = millis();

	} else if( iscommand( inptr, F("de"))) { 	//	dump eeprom
		uint8_t		b;
		uint16_t	address = getintparam( inptr );
		uint16_t	count = getintparam( inptr );
		for( uint16_t offset = 0; offset < count; ++offset )
		{
			if( offset && !( offset & 0xf ))	Serial.println();
			else if( offset ) Serial.print(' ');
			b = g_db.read_byte( address + offset );
			Serial.print( halfbytetohex( b >> 4));
			Serial.print( halfbytetohex( b & 0xf));
			delay(10);
		}

//	} else if( iscommand( inptr, F("dep"))) {	//dump eeprom paged
//		uint8_t		buffer[16];
//		uint16_t	address = getintparam( inptr );
//		uint16_t	count = getintparam( inptr );
//		if(address == 0xffff) address = 0;
//		address &= 0xfff0;
//
//		for( uint32_t pageoffset = 0; pageoffset < count; pageoffset += 16 )
//		{
//			Serial.println();
//			char	*bp((char*)buffer);
//			bp += uitohex( bp, pageoffset, 4 );
//			*bp++ = ' ';
//			*bp = 0;
//			Serial.print((char*) buffer );
//
//			g_db.read_page( address + pageoffset, buffer, 16 );
//			for( uint8_t offset=0; offset < 16; ++offset ) {
//				if( offset) Serial.print(' ');
//				Serial.print( halfbytetohex( buffer[offset] >> 4));
//				Serial.print( halfbytetohex( buffer[offset] & 0xf));
//			}
//		}

//	} else if( iscommand( inptr, F("se"))) {	//	set eeprom
//		uint16_t	address = getintparam( inptr );
//		uint16_t	value = getintparam( inptr );
//		g_db.write_byte( address, value );
//
	} else if( iscommand( inptr, F("ge"))) {	//	get eeprom
		uint16_t	address = getintparam( inptr );
		uint8_t		value = g_db.read_byte( address );
		Serial.print( halfbytetohex( value >> 4 ));
		Serial.println( halfbytetohex( value & 0xf ));

//	} else if( iscommand( inptr, F("fe"))) {	//	fill eeprom <start> <value> <count>
//		uint16_t	address = getintparam( inptr );
//		uint8_t		value = getintparam( inptr );
//		uint16_t	count = getintparam( inptr );
//		g_db.fill_page( address, value, count );

	} else if( iscommand( inptr, F("ddb"))) {	//	dump database
		char recbuf[ INFORECORD_WIDTH + STATUSRECORD_WIDTH + 1 ];
		database::dbrecord	rec;
		uint16_t	from( getintparam( inptr ));
		uint16_t	to( getintparam( inptr ));
		if( from == 0xffff ) from = 0;
		if( to == 0xffff ) to = 1023;
		for( uint16_t code = from; code <= to; ++code ) {
			if( g_db.getParams( code, rec )) {
				rec.serialize( recbuf );
				Serial.println( recbuf );
			} else {
				Serial.println( F(ERRS "GETPARAMS" ));
			}
		}

	} else if( iscommand( inptr, F("dtdb"))) {	//	dump database
		char recbuf[ INFORECORD_WIDTH + STATUSRECORD_WIDTH + 1 ];
		database::dbrecord	rec;
		thindb				tdb( g_sd );
		uint16_t	from( getintparam( inptr ));
		uint16_t	to( getintparam( inptr ));
		if( from == 0xffff ) from = 0;
		if( to == 0xffff ) to = 1023;
		if( tdb.init()) {
			for( uint16_t code = from; code <= to; ++code ) {
				if( tdb.getParams( code, rec )) {
					rec.serialize( recbuf );
					Serial.println( recbuf );
				} else {
					Serial.println( F(ERRS "GETPARAMS" ));
				}
			}
		}

	} else if( iscommand( inptr, F("fdb"))) {	//	fill database
		database::dbrecord	rec = { 0, 0xfff, 0, 0xfff, 0x7f, database::dbrecord::UNKNOWN };

		for( int code = 0; code < 1024; ++code ) {
			g_db.setParams( code, rec );
		}

	} else if( iscommand( inptr, F("cdb"))) {	//	fill database
		database::dbrecord	rec = { 0, 0, 0, 0, 0, database::dbrecord::UNKNOWN };

		for( int code = 0; code < 1024; ++code ) {
			g_db.setParams( code, rec );
		}

	} else if( iscommand( inptr, F("ftdb"))) {	//	fill thin database
		database::dbrecord	rec = { 0, 0xfff, 0, 0xfff, 0x7f, database::dbrecord::UNKNOWN };
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

	} else if( iscommand( inptr, F("imp"))) {	//import thindb
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

	} else if( iscommand( inptr, F("dl"))) {	// dump log
		g_logger.dump( &Serial );

	} else if( iscommand( inptr, F("tl"))) {	// truncate log
		g_logger.truncate();

	} else {
		Serial.println( F(ERRS "CMD"));
	}

	g_inidx = 0;
}

//////////////////////////////////////////////////////////////////////////////
void printdatetime( bool yeardigits = 0, bool showdow = true )
{
#ifdef	TEST_DS3231
	static ts		prevt = {0,0,0,0,0,0,0,0,0,0};

	char	lcdbuffer[17];
	char	*lbp(lcdbuffer);

	g_clk.get( &g_dt );

#ifdef	TEST_LCD
	if( prevt.year != g_dt.year || prevt.mon != g_dt. mon || prevt.mday != g_dt.mday ) {
		g_lcd.setCursor(0,0);
		datetostring( lbp, g_dt.year, g_dt.mon, g_dt.mday, g_dt.wday, yeardigits, showdow, '.', '/' ); *lbp = 0;
		g_lcd.print( lcdbuffer );
	}

	if( prevt.hour != g_dt.hour || prevt.min != g_dt.min || prevt.sec != g_dt.sec ) {
		g_lcd.setCursor(0,1);
		lbp = lcdbuffer;
		timetostring( lbp, g_dt.hour, g_dt.min, g_dt.sec, ':' ); *lbp++ = 0;
		g_lcd.print( lcdbuffer);
	}

#else	//	TEST_LCD

	if( prevt.year != g_dt.year || prevt.mon != g_dt. mon || prevt.mday != g_dt.mday ||
		prevt.hour != g_dt.hour || prevt.min != g_dt.min || prevt.sec != g_dt.sec )
	{
		serialout( (uint16_t)g_dt.year, F("."));
		serialout( (uint16_t)g_dt.mon,F("."));
		serialout( (uint16_t)g_dt.mday, F("/" ),(uint16_t)g_dt.wday);
		serialout( F("    "), (uint16_t)g_dt.hour);
		serialout(F(":" ), (uint16_t)(g_dt.min));
		serialout(F(":" ), (uint16_t)(g_dt.sec));
		Serial.println();
	}
#endif	//	TEST_LCD

	prevt = g_dt;
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
			lbp += uitodec( lbp, pin, 2 ); *lbp++ = 0;
		} else {
			lcdbuffer[0] = ' '; lcdbuffer[1] = ' '; lcdbuffer[2] = 0;
		}
		g_lcd.setCursor(10,0);
		g_lcd.print( lcdbuffer );
		prevpin = pin;
	}
#endif	//	TEST_LCD
}

//////////////////////////////////////////////////////////////////////////////
void printpins( uint8_t status )
{
#ifdef TEST_LCD
	static uint8_t	prevps(0);

	if( prevps != status ) {
		g_lcd.setCursor( 8, 0 );
		for( uint8_t mask = 1; mask; mask <<= 1 ) {
			g_lcd.print( (status & mask) ? '*':'_' );
		}
		prevps = status;
	}
#endif	//	TEST_LCD
}

//////////////////////////////////////////////////////////////////////////////
void printloopstatus( bool ils, bool ols )
{
	static bool first(true), previls, prevols;

	if( first || ils != previls || ols != prevols )
	{
		g_lcd.setCursor( 14, 1 );
		g_lcd.print( ils ? '*':'_' );
		g_lcd.print( ols ? '*':'_' );
		prevols = ols;
		previls = ils;
		first = false;
	}
}

//////////////////////////////////////////////////////////////////////////////
void printcode( uint16_t code )
{
	char	lcdbuffer[5];
	char	*lbp(lcdbuffer);

	lbp += uitodec( lbp, code, 4);
	*lbp = 0;
	g_lcd.setCursor( 9, 1);
	g_lcd.print( lcdbuffer );
}

//////////////////////////////////////////////////////////////////////////////
void setrelays( uint8_t prev, uint8_t curr )
{
	g_outputs.write(g_pinmap[prev], RELAY_OFF);
	g_ioext.write(g_pinmap[prev], RELAY_OFF);
	g_outputs.write(g_pinmap[curr], RELAY_ON);
	g_ioext.write(g_pinmap[curr], RELAY_ON);
}

//////////////////////////////////////////////////////////////////////////////
void setup()
{
	Serial.begin( 115200 );
	delay(100);
	Serial.println();
	serialoutln(F( CMNTS "------------------ Setup ------------------"));
	delay(100);

	I2c.begin();
	I2c.timeOut(1000);
	g_lcd.init();                      // initialize the lcd
	Serial.println(F(CMNTS "LCD initialized."));

	g_lcd.backlight();
	g_lcd.print( freeMemory());

	g_outputs.init(g_alloutputpins, RELAY_OFF);

	pinMode( PIN_INNERLOOP, INPUT );
	digitalWrite( PIN_INNERLOOP, HIGH );	//	activate pullup;
	pinMode( PIN_OUTERLOOP, INPUT );
	digitalWrite( PIN_OUTERLOOP, HIGH );	//	activate pullup;

	g_clk.init( DS3231_INTCN );

	g_sd.begin( SS );
	g_db.init();
	g_logger.init();

	setup433();

	delay( 2000 );
	printdatetime();
}

//////////////////////////////////////////////////////////////////////////////
void loop()
{
#ifdef FAILSTATS
	static stats prevstats;
	static stats *ps;
	static unsigned long statsprinted(0);
#endif
	static uint16_t			s_id(0);

	if( getlinefromserial( g_inbuf, sizeof(g_inbuf), g_inidx )) {
		processInput();
	}

	printdatetime();

	if( g_pinindex != 0xff )
	{
		if( millis() - g_rtstart > g_rtdelay )
		{

			uint8_t previndex(g_pinindex++);
			if( g_pinindex >= 8 )
				g_pinindex = 0;
			uint8_t raw( 1 << g_pinindex );
			setrelays(previndex, g_pinindex);
			printpins( raw );
			g_rtstart += g_rtdelay;
			database::dbrecord	rec;
			g_db.getParams( s_id, rec );
			char recbuf[ DBRECORD_WIDTH + 1 ];
			uitodec(recbuf, s_id, 4);
			recbuf[4] = ' ';
			recbuf[5] = 0;
			Serial.print(recbuf);
			rec.serialize(recbuf);
			Serial.println(recbuf);
			if( ++s_id > 1023 ) s_id = 0;
		}
	}
	printloopstatus( digitalRead( PIN_INNERLOOP ) == LOOP_ACTIVE, digitalRead( PIN_OUTERLOOP ) == LOOP_ACTIVE );
	if( g_codeready ) {
		printcode( getid( g_code ));
		serialoutln( g_code, F(", "), getid( g_code ));
		g_logger.log(logwriter::DEBUG, g_dt, F("CODE"), getid(g_code), getbutton(g_code));
		g_codeready = false;
	}
#ifdef FAILSTATS
	else
	{
		unsigned long now = millis();
		ps = (stats*)&g_stats;
		if( !(prevstats == *ps) && now - statsprinted > 1000 )
		{
			statsprinted = now;
			String s( String( g_stats.startabort )
					+ String( " " ) + String( g_stats.dataabort )
					+ String( " " ) + String( g_stats.stopabort )
					+ String( " " ) + String( g_stats.stopdeltat )
			);
			Serial.println( s );
			prevstats = *ps;
		}
	}
#endif	//	FAILSTATS

	delay(10);
}
