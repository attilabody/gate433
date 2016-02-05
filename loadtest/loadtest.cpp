// Do not remove the include beSTATE2
#include "loadtest.h"

#include <TimerOne.h>

#define BAUDRATE 115200
#define ITEMCOUNT(A) (sizeof(A)/sizeof(A[0]))
#define CYCLETIME 440

const uint8_t g_outPin( 3 );
const uint8_t g_ledPin( LED_BUILTIN );

volatile bool	g_inputavail( false );
unsigned int	g_input;

#define ST_WHITE	HIGH
#define ST_BLACK	LOW
//#define ST_WHITE	LOW
//#define ST_BLACK	HIGH

void isr();

void setup()
{
	pinMode(g_ledPin, OUTPUT);
	pinMode(g_outPin, OUTPUT);

	Timer1.initialize( CYCLETIME );
	Timer1.attachInterrupt( isr );

	Serial.begin(BAUDRATE);
}

void isr()
{
	static char			curbit(-2), bitphase(0);
	static unsigned int	data, bitmask;
	static bool			bit;

	if( curbit == -1 )
	{					//	start bit
		if( g_inputavail )
		{
			digitalWrite( g_outPin, ST_WHITE );
			data = g_input;
			g_inputavail = false;
			curbit = bitphase = 0;
			bitmask = 1 << 11;
		}
	}
	else if( curbit >= 0 && curbit < 12 )
	{	//	data bits
		switch( bitphase ) {
		case 0:
			bit = ( data & bitmask ) != 0;
			digitalWrite( g_outPin, ST_BLACK );
			++bitphase;
			break;
		case 1:
			if( bit ) digitalWrite( g_outPin, ST_WHITE );
			++bitphase;
			break;
		case 2:
			if( !bit ) digitalWrite( g_outPin, ST_WHITE );
			bitmask >>= 1;
			bitphase = 0;
			++curbit;
			break;
		}
	}
	else
	{ 									//	stop bit
		if( !bitphase++ ) digitalWrite( g_outPin, ST_BLACK );
		if( bitphase == 37 ) curbit = -1;
	}
}


// The loop function is called in an endless loop
void loop()
{
	static bool ledStatus( true );

	for( unsigned int code = 0; code < (1<<12); code += 4 ) {
		if( !((code >> 2) & 0xf )) {
			digitalWrite( g_ledPin, ledStatus ? HIGH : LOW );
			ledStatus = !ledStatus;
		}
		for( int retry = 0; retry < 4; ++retry ) {
			while( g_inputavail );
			g_input = code;
			g_inputavail = true;
			Serial.println( code );
		}
	}
}
