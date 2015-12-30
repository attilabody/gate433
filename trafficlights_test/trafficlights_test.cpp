// Do not remove the include below
#include "Arduino.h"
#include <LiquidCrystal_I2C.h>
#include "config.h"
#include "interface.h"
#include "trafficlights.h"
#include "inductiveloop.h"

#define STEP_LEN 5000

struct serinit
{
	serinit( unsigned long baud = 115200) { Serial.begin( baud ); }
};// g_si;

enum STATUS { WAITSETTLE, NEEDCODE, PASS, RETREAT };

const uint8_t		g_innerpins[3] = { 4,5,6 };
const uint8_t		g_outerpins[3] = { 7,8,9 };
const char 			*g_phasenames[] = { "OFF", "NEEDCODE", "CONFLICT", "ACCEPTED", "WARNED", "DENIED", "PASS" };
char				g_buf[5];


char		g_inbuf[128+1];
uint16_t	g_inidx(0);
const char 		*g_commands[] = {
	  "sdt"		//set datetime
	, "ddb"		//dump db
	, "rs"		//relay stop
	, "rt"		//relay test
	, "bt"		//blink test
	, ""
};


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
#ifndef SIMPLE_TEST
void processInput();
#endif	//	SIMPLE_TEST

//////////////////////////////////////////////////////////////////////////////
void setup()
{
	Serial.begin( 115200 );
	g_lcd.init();
	g_lcd.backlight();
	g_indloop.update();
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
			Serial.print( g_phasenames[offset] ); Serial.print( " (" ); Serial.print(g_buf); Serial.println("), true");
			LCDprint( 0, LCDprint( 0, 0, g_phasenames[offset], 0, false ), ", true", 0, true);
			LCDprint( 1, 0, g_buf, 4, true );
			break;

		case PREP2_START ...PREP2_END:
			offset = phase - PREP2_START;
			rawcode = g_lights.set( (trafficlights::STATUS) (offset), false);
			uitohex( bufptr, rawcode, 4 );
			*bufptr = 0;
			Serial.print( g_phasenames[offset] ); Serial.print( " (" ); Serial.print(g_buf); Serial.println("), false");
			LCDprint( 0, LCDprint( 0, 0, g_phasenames[offset], 0, false ), ", false", 0, true);
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

	unsigned long currmillis( millis());
	g_lights.loop( currmillis );
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
	ilchanged = (ilstatus == ilstatussaved) && (ilconflict == ilconflictsaved );

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
			g_lights.set( trafficlights::NEEDCODE, inner );
			tlstatus = trafficlights::NEEDCODE;
			status = NEEDCODE;
		}
		break;

	case NEEDCODE:
		if( ilchanged ) {
			status = WAITSETTLE;
			break;
		}
		//TODO:
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
		if( ! ilchanged ) break;
		//TODO: lights
		if( ilstatus != (inner ? inductiveloop::INNER : inductiveloop::OUTER)) {
			if( ilstatus == inductiveloop::NONE ) {
				g_lights.set( trafficlights::OFF, inner );
				tlstatus = trafficlights::OFF;
			} else {
				g_lights.set( trafficlights::OFF, inner );
				tlstatus = trafficlights::OFF;
			}
		}
		break;
	}



	unsigned long curmillis( millis() );
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
	const char	*inptr( g_inbuf );
	char 		command( findcommand( inptr, g_commands ));

	serialoutln( CMNT, (uint16_t)command );

	switch( command ) {
	case 0:		//	sdt
		break;

	default:
		Serial.println( ERRS "CMD");
		break;
	}
	g_inidx = 0;
}

#endif	//	SIMPLE_TEST
