// Do not remove the include below
#include "Arduino.h"
#include "trafficlights.h"

#define STEP_LEN 5000

uint8_t	innerpins[3] = { 4,5,6 };
uint8_t	outerpins[3] = { 7,8,9 };

//trafficlight	g_innerlights;
//trafficlight	g_outerlights;
trafficlights	g_lights;
//The setup function is called once at startup of the sketch
void setup()
{
	Serial.begin( 115200 );
//	g_innerlights.init( innerpins, false );
//	g_outerlights.init( outerpins, false );
	g_lights.init( innerpins, outerpins, false, 500 );
}

// The loop function is called in an endless loop
void loop()
{
	static uint8_t	phase(0);
	static const uint16_t	testvals[] = { 0x0104, 0x1144, 0x0202, 0x2222, 0x0401, 0x4411 };
	static unsigned long	laststart( millis() - STEP_LEN - 1 );
	if( millis() - laststart  > STEP_LEN )
	{
		laststart = millis();
		Serial.println(phase);



//		if( phase < trafficlights::NUMSTATES - 1)
//			g_lights.set( (trafficlights::STATES) (phase+1), true );
//		else
//			g_lights.set( (trafficlights::STATES) 0, true);
//	OFF=0, NEEDCODE, CONFLICT, ACCEPTED, DENIED, PASS
		switch( phase )
		{
		case 0 ... 5:
			g_lights.set( testvals[phase], true );
			break;
		case 6 ... 11:
		g_lights.set( testvals[phase-6], false );
			break;

		case 12:
			g_lights.set( 0, false );
			break;

		default:
			break;
		}

		++phase;
	}

	unsigned long currmillis( millis());
//	g_innerlights.loop( currmillis );
//	g_outerlights.loop( currmillis );
	g_lights.loop( currmillis );
}
