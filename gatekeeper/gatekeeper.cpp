#include <Wire.h>
#include <ds3231.h>
#include <SdFat.h>
#include "config.h"
#include "globals.h"
#include "interface.h"
#include "serialbuf.h"
#include "decode433.h"
#include "gatehandler.h"
#include "intdb.h"
#include "sdfatlogwriter.h"

//////////////////////////////////////////////////////////////////////////////
void setuprelaypins( const uint8_t *pins, uint8_t size );
void processinput();
void printdatetime( bool shortyear = true, bool showdow = false );
void printdecision( char decision );
void printlastreceived();
int getlinefromfile( SdFile &f, char* buffer, uint8_t buflen );
void updatedow( ts &t );

//////////////////////////////////////////////////////////////////////////////
void setup()
{
	Serial.begin( BAUDRATE );
	delay(10);
	for( char c = 0; c < 79; ++c ) Serial.print('-');
	Serial.println();

#ifdef PIN_LED
	pinMode( PIN_LED, OUTPUT );
#endif

	g_lcd.init();		//	calls Wire.begin()
	g_lcd.backlight();

	if( g_sd.begin( SS ))
		g_dbinitfail = !g_db.init();

	g_lcd.print( F("DBinit ") );
	g_lcd.print( g_dbinitfail ? F("FAILURE!!") : F("SUCCESS!!") );
	delay(1000);
	g_lcd.clear();

	g_indloop.init( PIN_INNERLOOP, PIN_OUTERLOOP, LOW );
	g_lights.init( g_innerlightspins, g_outerlightspins, RELAY_ON == HIGH, 500 );
	setuprelaypins( g_otherrelaypins, sizeof(g_otherrelaypins));

#ifdef PIN_LED
	noInterrupts();
	// disable all interrupts
	TIMSK0 |= ( 1 << OCIE0A );  // enable timer compare interrupt
	interrupts();
	// enable all interrupts
#endif	//	PIN_LED

	setup433();

	DS3231_init( DS3231_INTCN );
	g_lastdtupdate = millis();
	DS3231_get( &g_t );
	updatedow( g_t );

	g_logger.init();
	g_logger.log( logwriter::INFO, g_t, -1, F("Restart") );
}

//////////////////////////////////////////////////////////////////////////////
void loop()
{
	static gatehandler	handler( g_db, g_lights, g_indloop, g_lcd, ENFORCE_POS, ENFORCE_DT );
	unsigned long		now( millis() );


	if( now - g_lastdtupdate > 950 )
	{
		ts	t;
		DS3231_get( &t );

		if( t.sec != g_t.sec ) {
			if( t.mday != g_t.mday ) {
				g_t = t;
				updatedow( g_t );
			} else {
				g_t.sec = t.sec;
				g_t.min = t.min;
				g_t.hour = t.hour;
			}
			g_lastdtupdate = now;
		}
	}

	if( getlinefromserial( g_iobuf, sizeof( g_iobuf ), g_inidx) )
		processinput();

	char decision( handler.loop( now ) );

	printdatetime( true, true );
	if( decision ) printdecision( decision );
	if( g_codedisplayed != g_lrcode ) {
		printlastreceived();
		g_logger.log( logwriter::INFO, g_t, g_lrcode, F("Code received"));
	}
}

#ifdef PIN_LED
//////////////////////////////////////////////////////////////////////////////
ISR( TIMER0_COMPA_vect ) {
	digitalWrite( PIN_LED, ( micros() - g_codetime < 500000 ) ? HIGH : LOW );
}
#endif

//////////////////////////////////////////////////////////////////////////////
void setuprelaypins( const uint8_t *pins, uint8_t size )
{
	while( size > 0 ) {
		pinMode( *pins, OUTPUT );
		digitalWrite( *pins++, RELAY_OFF );
		--size;
	}
}

//////////////////////////////////////////////////////////////////////////////
void processinput()
{
	const char	*inptr( g_iobuf );

	Serial.println( g_iobuf );

	if( iscommand( inptr, F("get"))) {
		database::dbrecord	rec;
		int 				id( getintparam( inptr ));
		if( id != -1 && g_db.getParams( id, rec )) {
			rec.serialize( g_iobuf );
			serialoutln( RESP, g_iobuf );
		} else Serial.println( F(ERRS "ERR"));

	} else if( iscommand( inptr, F("set"))) {
		database::dbrecord	rec;
		int 				id( getintparam( inptr ));
		if( id != -1 && rec.parse( inptr )) {
			if( g_db.setParams( id, rec ))
				Serial.println( F( RESPS "OK"));
			else Serial.println( F(ERRS "ERR"));
		} else Serial.println( F(ERRS "ERR"));

	} else if( iscommand( inptr, F("export"))) {
		thindb		tdb( g_sd );
		uint16_t	from( getintparam( inptr ));
		uint16_t	to( getintparam( inptr ));
		uint16_t	id;
		database::dbrecord	rec;
		if( from == 0xffff ) from = 0;
		if( to == 0xffff ) to = 1023;
		if( tdb.init()) {
			for( id = from; id <= to; ++id )
				if( g_db.getParams( id, rec ))
					tdb.setParams( id, rec );
		}
		if( id == to )  Serial.println(F(RESPS "OK"));
		else Serial.println( F(ERRS "ERR" ));

	} else if( iscommand( inptr, F("import"))) {
		thindb				tdb( g_sd );
		uint16_t			from( getintparam( inptr ));
		uint16_t			to( getintparam( inptr ));
		uint16_t			id;
		database::dbrecord	rec;
		if( from == 0xffff ) from = 0;
		if( to == 0xffff ) to = 1023;
		if( tdb.init()) {
			for( id = from; id <= to; ++id )
				if( tdb.getParams( id, rec ))
					g_db.setParams( id, rec );
		}
		if( id == to )  Serial.println(F(RESPS "OK"));
		else Serial.println( F(ERRS "ERR" ));

	} else if( iscommand( inptr, F("dump"))) {
		database::dbrecord	rec;
		uint16_t			start( getintparam( inptr ));
		uint16_t			count( getintparam( inptr ));
		uint16_t			id;
		if( start == 0xffff ) start = 0;
		if( count == 0xffff ) count = 1024 - start;
		for( id = start; id < start + count; ++id ) {
			if( g_db.getParams( id, rec )) {
				rec.serialize( g_iobuf );
				serialoutln( RESP, g_iobuf );
			} else break;
		}
		if( id == start + count ) Serial.println( RESP );
		else Serial.println( F(ERRS "ERR" ));

	} else if( iscommand( inptr, F("gdt"))) {	// get datetime
		serialout( RESP, (uint16_t)g_t.year, F("."));
		serialout( (uint16_t)g_t.mon,F("."));
		serialout( (uint16_t)g_t.mday, F("/" ),(uint16_t)g_t.wday);
		serialout( F("    "), (uint16_t)g_t.hour);
		serialout(F(":" ), (uint16_t)(g_t.min));
		serialoutln(F(":" ), (uint16_t)(g_t.sec));

	} else if( iscommand( inptr, F("ds"))) {	// dump shuffle
		SdFile	f;
		char buffer[16];
		if( f.open( "shuffle.txt", FILE_READ )) {
			while( getlinefromfile( f, buffer, sizeof(buffer)) != -1 )
				serialoutln( RESP, buffer );
			Serial.println( RESP );
			f.close();
		} else {
			Serial.println( F(ERRS "Cannot open file."));
		}

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
void printdatetime( bool shortyear, bool showdow )
{
	static ts		dispt = {0,0,0,0,0,0,0,0,0,0};

	char	lcdbuffer[13];
	char	*lbp(lcdbuffer);

	if( dispt.year != g_t.year || dispt.mon != g_t. mon || dispt.mday != g_t.mday ) {
		g_lcd.setCursor(0,0);
		datetostring( lbp, g_t.year, g_t.mon, g_t.mday, g_t.wday, 0, showdow, '.', '/' ); *lbp = 0;
		g_lcd.print( lcdbuffer );
	}

	if( dispt.hour != g_t.hour || dispt.min != g_t.min || dispt.sec != g_t.sec ) {
		g_lcd.setCursor(0,1);
		lbp = lcdbuffer;
		timetostring( lbp, g_t.hour, g_t.min, g_t.sec, ':' ); *lbp++ = 0;
		g_lcd.print( lcdbuffer);
	}

	dispt = g_t;
}

//////////////////////////////////////////////////////////////////////////////
void printdecision( char decision )
{
	char buf[5];
	char*bp(buf);

	uitodec( bp, g_code >> 2, 4);
	buf[4] = 0;
	g_lcd.setCursor( 10, 1 );
	g_lcd.print( decision );
	g_lcd.print( ' ' );
	g_lcd.print( buf );
}

//////////////////////////////////////////////////////////////////////////////
void printlastreceived()
{
	char buf[5];
	char*bp(buf);

	uitodec( bp, g_lrcode >> 2, 4);
	buf[4] = 0;
	g_lcd.setCursor( 12, 0 );
	g_lcd.print( buf );
	g_codedisplayed = g_lrcode;
}

//////////////////////////////////////////////////////////////////////////////
int getlinefromfile( SdFile &f, char* buffer, uint8_t buflen )
{
	uint32_t	curpos( f.curPosition());
	char *src( buffer );
	int rb( f.read( buffer, buflen - 1 ));
	if( !rb ) return -1;

	int ret(0);

	while( rb-- ) {
		char inc = *src;
		if( !inc || inc == '\r' || inc == '\n' ) {
			ret = src - buffer;
			*src++ = 0;
			if( inc && rb ) {
				inc = *src;
				if( !inc || inc == '\r' || inc == '\n' )
					++src;
			}
			break;
		}
		++src;
	}

	if( rb == -1 ) {
		*src = 0;
		ret = src - buffer;
	}
	f.seekSet( curpos + ( src - buffer ));
	return ret;
}


//////////////////////////////////////////////////////////////////////////////
void updatedow( ts &t )
{
	char buffer[12];
	const char *bp;
	uint8_t	month, day, dow;
	SdFile	f;

	if( f.open( "shuffle.txt", FILE_READ )) {
		while( getlinefromfile( f, buffer, sizeof(buffer)) != -1 ) {
			bp = buffer;
			if( (month = getintparam( bp, true, true, false )) != 0xff &&
				(day = getintparam( bp, true, true, false )) != 0xff &&
				(dow = getintparam( bp, true, true, false )) != 0xff &&
				month == g_t.mon && day == g_t.mday )
			{
				g_t.wday = dow;
				break;
			}
		}
		f.close();
	}
}
