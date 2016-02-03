#include <Wire.h>
#include <ds3231.h>
#include <SdFat.h>
#include <MemoryFree.h>
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
int getlinefromfile( SdFile &f, char* buffer, uint8_t buflen );
void updatedow( ts &t );

//////////////////////////////////////////////////////////////////////////////
void setup()
{
	Serial.begin( BAUDRATE );
	delay(10);
	Serial.print( CMNT );
	for( char c = 0; c < 79; ++c ) Serial.print('-');
	Serial.println();

	memset( &g_t, 0, sizeof( g_t ));

#ifdef PIN_LED
	pinMode( PIN_LED, OUTPUT );
#endif

	g_display.init();		//	calls Wire.begin()

	g_display.print( freeMemory() );

	g_sdpresent = g_sd.begin( SS );
	g_display.print( ' ' );
	g_display.print( g_sdpresent );

#ifdef USE_FLASHDB
	bool dbinitsucc = g_db.init();
#else
	bool dbinitsucc = sdinitsucc && g_db.init();
#endif

	g_display.print( ' ' );
	g_display.print( dbinitsucc );

	g_indloop.init( PIN_INNERLOOP, PIN_OUTERLOOP, LOW );
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

	bool loginitsucc = g_sdpresent && g_logger.init();
	g_display.print( ' ' );
	g_display.print( loginitsucc );


	delay(3000);

	g_logger.log( logwriter::WARNING, g_t, F("Restart"), -1 );
	g_display.clear();
	g_display.updatedt( g_t, 0xff );
	g_display.updateloopstatus( false, false );
	g_display.updatelastreceivedid( 9999 );
	g_display.updatelastdecision( 'X', 9999 );
}

//////////////////////////////////////////////////////////////////////////////
void loop()
{
	static gatehandler	handler( g_db, 500, g_indloop, g_display );
	unsigned long		now( millis() );
	byte				dtcmask(0);


	if( now - g_lastdtupdate > 950 )
	{
		ts	t;
		DS3231_get( &t );

		if( t.sec != g_t.sec ) {
			dtcmask |= 1;
			if( t.min != g_t.min ) {
				dtcmask |= 2;
				if( t.hour != g_t.hour ) {
					dtcmask |= 4;
					if( t.mday != g_t.mday ) {
						dtcmask |= 8;
						if( t.mon != g_t.mon ) {
							dtcmask |= 0x10;
							if( t.year != g_t.year )
								dtcmask |= 0x20;
		}}}}}

		if( dtcmask & 0x38 ) {
				g_t = t;
				updatedow( g_t );
		} else if( dtcmask &7) {
			g_t.sec = t.sec;
			g_t.min = t.min;
			g_t.hour = t.hour;
		}
		g_lastdtupdate = now;
		g_display.updatedt( g_t, dtcmask );
	}

	if( getlinefromserial( g_iobuf, sizeof( g_iobuf ), g_inidx) )
		processinput();

	handler.loop( now );

	if( g_codedisplayed != g_lrcode ) {
		g_display.updatelastreceivedid( getid( g_lrcode ));
		g_logger.log( logwriter::DEBUG, g_t, F("New code received"), g_lrcode >> 2);
		g_codedisplayed = g_lrcode;
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
const char PROGMEM CMD_GET[] = "get";
const char PROGMEM CMD_SET[] = "set";
const char PROGMEM CMD_EXP[] = "exp";
const char PROGMEM CMD_IMP[] = "imp";
const char PROGMEM CMD_DMP[] = "dmp";
const char PROGMEM CMD_CS[] = "cs";
const char PROGMEM CMD_GDT[] = "gdt";
const char PROGMEM CMD_SDT[] = "sdt";
const char PROGMEM CMD_DS[] = "ds";
const char PROGMEM CMD_DL[] = "dl";
const char PROGMEM CMD_TL[] = "tl";

//////////////////////////////////////////////////////////////////////////////
void processinput()
{
	const char	*inptr( g_iobuf );

	Serial.print( CMNT );
	Serial.println( g_iobuf );

	if( iscommand( inptr, CMD_GET )) {	//	get
		database::dbrecord	rec;
		int 				id( getintparam( inptr ));
		if( id != -1 && g_db.getParams( id, rec )) {
			rec.serialize( g_iobuf );
			serialoutln( RESP, g_iobuf );
		} else Serial.println( F(ERRS "ERR"));

	} else if( iscommand( inptr, CMD_SET )) {	//	set
		database::dbrecord	rec;
		int 				id( getintparam( inptr ));
		if( id != -1 && rec.parse( inptr )) {
			if( g_db.setParams( id, rec ))
				Serial.println( F( RESPS "OK"));
			else Serial.println( F(ERRS "ERR"));
		} else Serial.println( F(ERRS "ERR"));

	} else if( iscommand( inptr, CMD_EXP )) {	//	export
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
		if( id == to + 1 )  Serial.println(F(RESPS "OK"));
		else Serial.println( F(ERRS "ERR" ));

	} else if( iscommand( inptr, CMD_IMP)) {	//	import
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
		if( id == to + 1 )  Serial.println(F(RESPS "OK"));
		else Serial.println( F(ERRS "ERR" ));

	} else if( iscommand( inptr, CMD_DMP )) {	//	dump
		database::dbrecord	rec;
		uint16_t			from( getintparam( inptr ));
		uint16_t			to( getintparam( inptr ));
		uint16_t			id;

		g_iobuf[3] = ' ';
		if( from == 0xffff ) from = 0;
		if( to == 0xffff ) to = 1023;
		for( id = from; id <= to; ++id ) {
			if( g_db.getParams( id, rec )) {
				uitohex( g_iobuf, id, 3 );
				rec.serialize( g_iobuf + 4 );
				serialoutln( RESP, g_iobuf );
			} else break;
		}
		if( id == to + 1 ) Serial.println( RESP );
		else Serial.println( F(ERRS "ERR" ));

	} else if( iscommand( inptr, CMD_CS )) {	//	clear statuses
		uint16_t			from( getintparam( inptr ));
		uint16_t			to( getintparam( inptr ));
		uint16_t			id;

		if( from == 0xffff ) from = 0;
		if( to == 0xffff ) to = 1023;
		for( id = from; id <= to; ++id ) {
			g_db.setStatus( id, database::dbrecord::UNKNOWN );
		}

	} else if( iscommand( inptr, CMD_GDT )) {	// get datetime
		serialout( RESP, (uint16_t)g_t.year, F("."));
		serialout( (uint16_t)g_t.mon,F("."));
		serialout( (uint16_t)g_t.mday, F("/" ),(uint16_t)g_t.wday);
		serialout( F("    "), (uint16_t)g_t.hour);
		serialout(F(":" ), (uint16_t)(g_t.min));
		serialoutln(F(":" ), (uint16_t)(g_t.sec));

	} else if( iscommand( inptr, CMD_SDT )) {	//	set datetime
		ts	t;
		if( parsedatetime( t, inptr )) {
			DS3231_set( t );
			Serial.println( F(RESPS "OK"));
		} else
			Serial.println( F(ERRS " (DATETIMEFMT)"));

	} else if( iscommand( inptr, CMD_DS )) {	// dump shuffle
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

	} else if( iscommand( inptr, CMD_DL )) {	// dump log
		g_logger.dump( &Serial );

	} else if( iscommand( inptr, CMD_TL )) {	// truncate log
		g_logger.truncate();

	} else {
		Serial.println( F(ERRS "CMD"));
	}

	g_inidx = 0;
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

	if( !g_sdpresent ) return;

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
