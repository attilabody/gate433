// Do not remove the include beSTATE2
#include <Arduino.h>
#include <TimerOne.h>
#include "loadtest.h"


#define BAUDRATE 115200
#define ITEMCOUNT(A) (sizeof(A)/sizeof(A[0]))
#define CYCLETIME 440

const uint8_t g_outPin( 3 );
const uint8_t g_ledPin( LED_BUILTIN );
const uint8_t g_loopin( 4 );
const uint8_t g_loopout( 5 );

#define	LOOP_ACTIVE	LOW
#define LOOP_INACTIVE HIGH

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
	pinMode(g_loopin, OUTPUT);
	pinMode(g_loopout, OUTPUT);

	digitalWrite( g_loopin, LOOP_INACTIVE );
	digitalWrite( g_loopout, LOOP_INACTIVE );

	Timer1.initialize( CYCLETIME );
	Timer1.attachInterrupt( isr );

	Serial.begin(BAUDRATE);
}

void printio( bool inner )
{
	Serial.print( inner ? F("inner ") : F("outer ") );
}

const char PROGMEM g_innerstr[] = "inner";
const char PROGMEM g_outerstr[] = "outer";

// The loop function is called in an endless loop
void loop()
{
	static bool ledStatus( true );
	static bool direction;

	for( unsigned int code = 0; code < (1024); ++code )
	{
		Serial.print( F("<-------> Starting loop "));
		Serial.println( code );
		bool dir(((bool)(code & 1)) == direction);

		if( !(code & 0xf )) {
			digitalWrite( g_ledPin, ledStatus ? HIGH : LOW );
			ledStatus = !ledStatus;
		}


		digitalWrite( dir ? g_loopin : g_loopout, LOOP_ACTIVE );
		Serial.print(F("Activating "));
//		Serial.print(F( dir ? g_innerstr : g_outerstr ));
		printio(dir);
		Serial.println(F("loop."));
		delay( 2000 );

		Serial.println(F("Sending code."));
		for( int retry = 0; retry < 4; ++retry ) {
			while( g_inputavail );
			g_input = code << 2;
			g_inputavail = true;
		}
		Serial.println(F("Code sent."));
		delay( 2000 );

		Serial.print(F("Activating "));
		printio(!dir);
		Serial.println(F("loop."));
		digitalWrite( !dir ? g_loopin : g_loopout, LOOP_ACTIVE );
		delay( 100 );
		Serial.print(F("Deactivating "));
		printio(dir);
		Serial.println(F("loop."));
		digitalWrite( dir ? g_loopin : g_loopout, LOOP_INACTIVE );
		delay( 2000 );
		Serial.print(F("Deactivating "));
		printio(!dir);
		Serial.println(F("loop."));
		digitalWrite( !dir ? g_loopin : g_loopout, LOOP_INACTIVE );
		delay( 5000 );
	}
	direction = ! direction;
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


