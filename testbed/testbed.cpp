// Do not remove the include below
#include "Arduino.h"
#include <LiquidCrystal_I2C.h>
#include <SdFat.h>
#include <MemoryFree.h>
#include "config.h"
#include "interface.h"
#include "trafficlights.h"
#include "inductiveloop.h"
#include "decode433.h"
#include "intdb.h"

#define STEP_LEN 5000

//struct serinit
//{
//	serinit( unsigned long baud = 115200) { Serial.begin( baud ); }
//} g_si;

enum STATUS { WAITSETTLE, CODEWAIT, PASS, RETREAT };

const uint8_t		g_innerpins[3] = { 0,1,2 };
const uint8_t		g_outerpins[3] = { 4,7,6 };
const char 			*g_tlstatusnames[] = { "OFF", "CODEW", "CONFL", "ACC", "WARN", "DENY", "PASS" };
const char 			*g_statusnames[] = { "WAITS", "CODEW", "PASS", "RETR" };
const char			*g_ilstatusnames[] = { "NONE", "IN", "OUT" };
char				g_buf[5];
char				g_inbuf[64+1];
uint16_t			g_inidx(0);
const char 			*g_commands[] = {
	  "inj"	//inj <int> simulate (inject) incoming code
	, "get"	//get <int> dump data for a code
	, "set"		//set data for a code
	, ""
};

SdFat				g_sd;
intdb				g_db( g_sd, false );
File				g_log;
LiquidCrystal_I2C 	g_lcd(LCD_ADDRESS, LCD_WIDTH, LCD_HEIGHT);
inductiveloop		g_indloop( PIN_INNERLOOP, PIN_OUTERLOOP, LOW );
trafficlights		g_lights( g_innerpins, g_outerpins, false, 500 );

#ifdef SIMPLE_TEST
const uint16_t		g_testvals[] = { 0x0104, 0x1144, 0x0202, 0x2222, 0x0401, 0x4411 };
#define PREP_LEN (sizeof(g_phasenames)/sizeof(g_phasenames[0]))
#define PREP1_START 0
#define PREP1_END (PREP1_START + PREP_LEN - 1)
#define PREP2_START (PREP1_START + PREP_LEN)
#define PREP2_END (PREP2_START + PREP_LEN - 1)
#define RAW_LEN	(sizeof(g_testvals)/sizeof(g_testvals[0]))
#define RAW1_START (PREP2_END + 1)
#define RAW1_END (RAW1_START + RAW_LEN - 1)
#define RAW2_START (RAW1_START + RAW_LEN)
#define RAW2_END (RAW2_START + RAW_LEN - 1)
#endif	//	SIMPLE_TEST

uint8_t LCDprint( uint8_t row, uint8_t startpos, const char *buffer, uint8_t len, bool erase );
void inline printfree()
{
	serialoutln( F("Free memory: "), freeMemory() );
}
#ifndef SIMPLE_TEST
void processInput();
#endif	//	SIMPLE_TEST

//////////////////////////////////////////////////////////////////////////////
void setup()
{
	Serial.begin( 115200 );
	printfree();
	delay(100);
	Serial.println( F("Initilalizing database..."));
	delay(1000);
	g_sd.begin( SS );
	if( !g_db.init() ) {
		Serial.println( F("Database initialization FAILED!") );
		for(;;);
	} else {
		Serial.println( F("Database initialization SUCCEEDED!") );
	}
	printfree();
	g_log = g_sd.open( "log.txt", FILE_WRITE );
	if( !g_log )
		Serial.println(F("Logfile creation failed!"));
	g_lcd.init();
	g_lcd.backlight();
	g_indloop.update();
	printfree();
	Serial.println( F("setup() finished."));
}

//////////////////////////////////////////////////////////////////////////////
void loop()
{
#ifdef SIMPLE_TEST
	static uint8_t			phase(0);
	static unsigned long	laststart( millis() - STEP_LEN - 1 );
	char					*bufptr;
	uint8_t					offset;
	uint16_t				rawcode;

	if( millis() - laststart  > STEP_LEN )
	{
		laststart = millis();
		Serial.println(phase);
		bufptr = g_buf;

		switch( phase )
		{
		case PREP1_START ... PREP1_END:
			offset = phase - PREP1_START;
			rawcode = g_lights.set( (trafficlights::STATUS) (offset), true);
			uitohex( bufptr, rawcode, 4 );
			*bufptr = 0;
			Serial.print( g_tlstatusnames[offset] ); Serial.print( " (" ); Serial.print(g_buf); Serial.println("), true");
			LCDprint( 0, LCDprint( 0, 0, g_tlstatusnames[offset], 0, false ), ", true", 0, true);
			LCDprint( 1, 0, g_buf, 4, true );
			break;

		case PREP2_START ...PREP2_END:
			offset = phase - PREP2_START;
			rawcode = g_lights.set( (trafficlights::STATUS) (offset), false);
			uitohex( bufptr, rawcode, 4 );
			*bufptr = 0;
			Serial.print( g_tlstatusnames[offset] ); Serial.print( " (" ); Serial.print(g_buf); Serial.println("), false");
			LCDprint( 0, LCDprint( 0, 0, g_tlstatusnames[offset], 0, false ), ", false", 0, true);
			LCDprint( 1, 0, g_buf, 4, true );
			break;

		case RAW1_START ... RAW1_END:
			g_lights.set( g_testvals[phase-RAW1_START], true );
			uitohex( bufptr, g_testvals[phase-RAW1_START], 4 );
			*bufptr = 0;
			Serial.print( g_buf ); Serial.println(", true");
			LCDprint( 0, LCDprint( 0, 0, g_buf, 4, false),", true", 0, true );
			LCDprint( 1, 0, " ", 1, true );
			break;

		case RAW2_START ... RAW2_END:
			g_lights.set( g_testvals[phase-RAW2_START], false );
			uitohex( bufptr, g_testvals[phase-RAW2_START], 4 );
			*bufptr = 0;
			Serial.print( g_buf ); Serial.println(", false");
			LCDprint( 0, LCDprint( 0, 0, g_buf, 4, false),", false", 0, true );
			LCDprint( 1, 0, " ", 1, true );
			break;

		case RAW2_END + 1:
			g_lights.set( 0, false );
			LCDprint( 0, 0, " ", 1, true );
			LCDprint( 1, 0, " ", 1, true );
			break;

		default:
			phase = -1;
			break;
		}

		++phase;
	}
#else	//	SIMPLE_TEST
	static STATUS					status( WAITSETTLE );
	static trafficlights::STATUS	tlstatus(trafficlights::OFF );
	static inductiveloop::STATUS	ilstatussaved( inductiveloop::NONE );
	static bool						ilconflictsaved( false );
	static bool						inner;

	inductiveloop::STATUS			ilstatus;
	bool							ilconflict, ilchanged;

	if( getlinefromserial( g_inbuf, sizeof(g_inbuf), g_inidx )) {
		processInput();
	}

	ilconflict = g_indloop.update( ilstatus );
	ilchanged = (ilstatus != ilstatussaved) || (ilconflict != ilconflictsaved );

	switch( status )
	{
	case WAITSETTLE:
		if( !ilchanged ) break;
		if( ilstatus == inductiveloop::NONE && tlstatus != trafficlights::OFF ) {
			g_lights.set( trafficlights::OFF, inner );
			tlstatus = trafficlights::OFF;
			break;
		} else if( ilconflict ) {
			inner = ilstatus == inductiveloop::INNER;
			g_lights.set( trafficlights::CONFLICT, inner );
			tlstatus = trafficlights::CONFLICT;
			break;
		} else if( ilstatus != inductiveloop::NONE ) {
			inner = ilstatus == inductiveloop::INNER;
			g_lights.set( trafficlights::CODEWAIT, inner );
			tlstatus = trafficlights::CODEWAIT;
			status = CODEWAIT;
		}
		break;

	case CODEWAIT:
		if( ilchanged ) {
			if( ilconflict ) {
				g_lights.set( trafficlights::CONFLICT, inner );
				tlstatus = trafficlights::CONFLICT;
				status = WAITSETTLE;
			} else if( ilstatus == inductiveloop::NONE ) {
				g_lights.set( trafficlights::OFF, inner );
				tlstatus = trafficlights::OFF;
				status = WAITSETTLE;
			} else {
				g_lights.set( trafficlights::CODEWAIT, inner );
				tlstatus = trafficlights::CODEWAIT;
			}
			break;
		}
		if( g_codeready ) {
			if( g_code & 1 ) {
				g_lights.set( trafficlights::DENIED, inner );
				tlstatus = trafficlights::DENIED;
				status = RETREAT;
			} else {
				g_lights.set( trafficlights::ACCEPTED, inner );
				tlstatus = trafficlights::ACCEPTED;
				status = PASS;
			}
			g_codeready = false;
		}
		break;

	case PASS:
		if( !ilchanged ) break;
		if( ilconflict ) {
			g_lights.set( trafficlights::PASS, inner );
			tlstatus = trafficlights::PASS;
		} else if( ilstatus == inductiveloop::NONE ) {
			g_lights.set( trafficlights::OFF, inner );
			tlstatus = trafficlights::OFF;
			status = WAITSETTLE;
		}
		break;

	case RETREAT:
		if( !ilchanged ) break;
		//TODO: lights
		if( ilstatus != (inner ? inductiveloop::INNER : inductiveloop::OUTER)) {
			if( ilstatus == inductiveloop::NONE ) {
				g_lights.set( trafficlights::OFF, inner );
				tlstatus = trafficlights::OFF;
				status = WAITSETTLE;
			} else {
				g_lights.set( trafficlights::CODEWAIT, !inner );
				tlstatus = trafficlights::CODEWAIT;
				status = CODEWAIT;
			}
		}
		break;
	}
#ifdef VERBOSE
	if( ilchanged || statussaved != status || tlstatussaved != tlstatus || innersaved != inner )
	{
		Serial.println("--------------------------------------------");
		if( ilstatussaved != ilstatus )
			serialoutln( F("Induction loop state changed from "), g_ilstatusnames[ilstatussaved], " to ", g_ilstatusnames[ilstatus] );
		if( ilconflictsaved != ilconflict )
			serialoutln( F("Conflict changed from "), ilconflictsaved ? "true" : "false", " to ", ilconflict ? "true" : "false" );
		if( statussaved != status )
			serialoutln( F("Status changed from "), g_statusnames[statussaved], " to ", g_statusnames[status]);
		if( tlstatussaved != tlstatus )
			serialoutln( F("Traffic lights Status changed from "), g_tlstatusnames[tlstatussaved], innersaved ? " (true)" : " (false)",
					" to ", g_tlstatusnames[tlstatus], inner ? " (true)" : " (false)");
		if( innersaved != inner )
			serialoutln( F("Inner changed from "), innersaved ? "true" : "false", " to ", inner ? "true" : "false" );
	}
#endif	//	VERBOSE
	ilstatussaved = ilstatus;
	ilconflictsaved = ilconflict;

	unsigned long currmillis( millis());
	g_lights.loop( currmillis );
#endif	//	SIMPLE_TEST
}

//////////////////////////////////////////////////////////////////////////////
uint8_t LCDprint( uint8_t row, uint8_t startpos, const char *buffer, uint8_t len, bool erase )
{
	g_lcd.setCursor( startpos, row );

	g_lcd.print( buffer );
	if( !len ) len = strlen( buffer );
	if( erase )
		for( ; len < LCD_WIDTH; ++ len )
			g_lcd.print( ' ' );
	return startpos + len;
}

#ifndef SIMPLE_TEST
//////////////////////////////////////////////////////////////////////////////
void processInput()
{
	const char			*inptr( g_inbuf );
	char 				command( findcommand( inptr, g_commands ));
	long				code;
	database::dbrecord	rec;
	char 				recbuf[ INFORECORD_WIDTH + STATUSRECORD_WIDTH + 1 ];

	serialoutln( F(CMNTS "Command ID: "), (uint16_t)command );
	printfree();

	switch( command ) {
	case 0:		//	inj
		code = getintparam( inptr );
		if( code != -1 ) {
			g_code = code;
			g_codeready = true;
		}
		break;
	case 1:		//get
		code = getintparam( inptr );
		if( code != -1 ) {
			g_db.getParams( code, rec );
			rec.serialize( recbuf );
			Serial.println( recbuf );
		}
		break;

	case 2:		//set 0 0 59f 0 59f 7f 0
		code = getintparam( inptr );
		if( code != -1 && rec.parse( inptr )) {
			g_db.setParams( code, rec );
		}
		break;

	default:
		Serial.println( ERRS "CMD");
		break;
	}
	g_inidx = 0;
}

#endif	//	SIMPLE_TEST
