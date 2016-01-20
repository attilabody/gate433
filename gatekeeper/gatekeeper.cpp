#include <Wire.h>
#include <ds3231.h>
#include "config.h"
#include "globals.h"
#include "interface.h"
#include "serialbuf.h"
#include "decode433.h"
#include "gatehandler.h"
#include "intdb.h"

//////////////////////////////////////////////////////////////////////////////
void setuprelaypins( const uint8_t *pins, uint8_t size );
void processinput();

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

	g_lcd.init();
	g_lcd.backlight();

	if( g_sd.begin( SS ))
		g_dbinitfail = !g_db.init();

	g_lcd.print( F("DBinit ") );
	g_lcd.print( g_dbinitfail ? F("FAILURE!!") : F("SUCCESS!!") );
	delay(3000);
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
}

//////////////////////////////////////////////////////////////////////////////
void loop()
{
	if( getlinefromserial( g_iobuf, sizeof(g_iobuf ), g_inidx) )
		processinput();

	static gatehandler				handler( g_db, g_lights, g_indloop, g_lcd, ENFORCE_POS, ENFORCE_DT );

	handler.loop( millis() );
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

	if( iscommand( inptr, F("get"))) {
		database::dbrecord	rec;
		int 				id( getintparam( inptr ));
		if( id != -1 && g_db.getParams( id, rec )) {
			rec.serialize( g_iobuf );
			Serial.print( F(RESPS) );
			Serial.println( g_iobuf );
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
				Serial.print( F(RESPS) );
				Serial.println( g_iobuf );
			} else break;
		}
		if( id == start + count ) Serial.println(F(RESPS "OK"));
		else Serial.println( F(ERRS "ERR" ));

	} else {
		Serial.println( F(ERRS "CMD"));
	}

	g_inidx = 0;
}
