// Do not remove the include below
#include "trafficlights_test.h"
#include "trafficlights.h"

#define STEP_LEN 5000

uint8_t	innerpins[3] = { 4,5,6 };
uint8_t	outerpins[3] = { 7,8,9 };

trafficlight	g_innerlights;
trafficlight	g_outerlights;
//The setup function is called once at startup of the sketch
void setup()
{
	Serial.begin( 115200 );
	g_innerlights.init( innerpins, false );
	g_outerlights.init( outerpins, false );
}

// The loop function is called in an endless loop
void loop()
{
	static uint8_t	phase(0);
	static unsigned long	laststart( millis() - STEP_LEN - 1 );
	if( millis() - laststart  > STEP_LEN )
	{
		laststart = millis();
		Serial.println(phase);

		switch( phase ) {
		case 0:
			g_innerlights.set(true, 500, true, 400, true, 300 );
			g_outerlights.set(true, 300, true, 400, true, 500 );
			break;

		case 1:	//NEEDCODE
			g_innerlights.set( false, 0, true, 0, false, 0 );
			g_outerlights.set( true, 0, false, 0, false, 0 );
			break;

		case 2:	//CONFLICT
			g_innerlights.set( false, 0, true, 500, false, 0 );
			g_outerlights.set( true, 500, false, 0, false, 0 );
			break;

		case 3:	//OPENING
			g_innerlights.set( false, 0, false, 0, true, 0 );
			g_outerlights.set( true, 0, false, 0, false, 0 );
			break;

		case 4:	//PASSTHRU + PASSED
			g_innerlights.set( true, 0, false, 0, false, 0 );
			g_outerlights.set( true, 0, false, 0, false, 0 );
			break;

		case 5:
			g_innerlights.set( false, 0, false, 0, false, 0 );
			g_outerlights.set( false, 0, false, 0, false, 0 );
			break;

		default:
			break;
		}

		++phase;
	}

	unsigned long currmillis( millis());
	g_innerlights.loop( currmillis );
	g_outerlights.loop( currmillis );
}
