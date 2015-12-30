// Do not remove the include below
#include "Arduino.h"
#include <LiquidCrystal_I2C.h>
#include "config.h"
#include "interface.h"
#include "trafficlights.h"

#define STEP_LEN 5000

struct serinit
{
	serinit( unsigned long baud = 115200) { Serial.begin( baud ); }
};// g_si;

const uint8_t		g_innerpins[3] = { 4,5,6 };
const uint8_t		g_outerpins[3] = { 7,8,9 };
const uint16_t		g_testvals[] = { 0x0104, 0x1144, 0x0202, 0x2222, 0x0401, 0x4411 };
const char 			*g_phasenames[] = { "OFF", "NEEDCODE", "CONFLICT", "ACCEPTED", "WARNED", "DENIED", "PASS" };
char				g_buf[32];
LiquidCrystal_I2C 	g_lcd(LCD_ADDRESS, LCD_WIDTH, LCD_HEIGHT);

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

trafficlights	g_lights( g_innerpins, g_outerpins, false, 500 );


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

//The setup function is called once at startup of the sketch
void setup()
{
	Serial.begin( 115200 );
	g_lcd.init();
	g_lcd.backlight();
}

// The loop function is called in an endless loop
void loop()
{
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
			rawcode = g_lights.set( (trafficlights::STATES) (offset), true);
			uitohex( bufptr, rawcode, 4 );
			*bufptr = 0;
			Serial.print( g_phasenames[offset] ); Serial.print( " (" ); Serial.print(g_buf); Serial.println("), true");
			LCDprint( 0, LCDprint( 0, 0, g_phasenames[offset], 0, false ), ", true", 0, true);
			LCDprint( 1, 0, g_buf, 4, true );
			break;

		case PREP2_START ...PREP2_END:
			offset = phase - PREP2_START;
			rawcode = g_lights.set( (trafficlights::STATES) (offset), false);
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
}
