// Do not remove the include below
#include "Arduino.h"
#include "interface.h"
#include "trafficlights.h"

#define STEP_LEN 5000

struct serinit
{
	serinit( unsigned long baud = 115200) { Serial.begin( baud ); }
};// g_si;

const uint8_t	g_innerpins[3] = { 4,5,6 };
const uint8_t	g_outerpins[3] = { 7,8,9 };
const uint16_t	g_testvals[] = { 0x0104, 0x1144, 0x0202, 0x2222, 0x0401, 0x4411 };
const char 		*g_phasenames[] = { "OFF", "NEEDCODE", "CONFLICT", "ACCEPTED", "DENIED", "PASS" };
char			g_buf[32];

#define PREP_LEN 6
#define PREP1_START 0
#define PREP1_END (PREP1_START + PREP_LEN - 1)
#define PREP2_START (PREP1_START + PREP_LEN)
#define PREP2_END (PREP2_START + PREP_LEN - 1)
#define RAW_LEN	6
#define RAW1_START (PREP2_START + PREP_LEN)
#define RAW1_END (RAW1_START + RAW_LEN - 1)
#define RAW2_START (RAW1_START + RAW_LEN)
#define RAW2_END (RAW2_START + RAW_LEN - 1)


//The setup function is called once at startup of the sketch
void setup()
{
	Serial.begin( 115200 );
//	g_lights.init( g_innerpins, g_outerpins, false, 500 );
}

// The loop function is called in an endless loop
void loop()
{
	static trafficlights	lights( g_innerpins, g_outerpins, false, 500 );
	static uint8_t			phase(0);
	static unsigned long	laststart( millis() - STEP_LEN - 1 );
	char					*bufptr;

	if( millis() - laststart  > STEP_LEN )
	{
		laststart = millis();
		Serial.println(phase);

		switch( phase )
		{
		case PREP1_START ... PREP1_END:
			lights.set( (trafficlights::STATES) (phase-PREP1_START), true);
			Serial.print( g_phasenames[phase-PREP1_START] ); Serial.println(", true");
			break;

		case PREP2_START ...PREP2_END:
			lights.set( (trafficlights::STATES) (phase-PREP2_START), false);
			Serial.print( g_phasenames[phase-PREP2_START] ); Serial.println(", false");
			break;

		case RAW1_START ... RAW1_END:
			lights.set( g_testvals[phase-RAW1_START], true );
			bufptr = g_buf;
			uitohex( bufptr, g_testvals[phase-RAW1_START], 4 );
			*bufptr = 0;
			Serial.print( g_buf ); Serial.println(", true");
			break;

		case RAW2_START ... RAW2_END:
			lights.set( g_testvals[phase-RAW2_START], false );
			bufptr = g_buf;
			uitohex( bufptr, g_testvals[phase-RAW2_START], 4 );
			*bufptr = 0;
			Serial.print( g_buf ); Serial.println(", false");
			break;

		case RAW2_END + 1:
			lights.set( 0, false );
			break;

		default:
			phase = -1;
			break;
		}

		++phase;
	}

	unsigned long currmillis( millis());
	lights.loop( currmillis );
}
