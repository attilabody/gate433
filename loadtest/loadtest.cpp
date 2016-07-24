// Do not remove the include beSTATE2
#include <Arduino.h>
#include "config.h"
#include <TimerOne.h>
#include <commsyms.h>
#include <toolbox.h>
#include "loadtest.h"


#define ITEMCOUNT(A) (sizeof(A)/sizeof(A[0]))
#define CYCLETIME 440

const uint8_t g_outPin( A0 );
const uint8_t g_ledPin( LED_BUILTIN );
const uint8_t g_loopin( 4 );
const uint8_t g_loopout( 5 );

char			g_iobuf[64];
uint8_t			g_inidx(0);

#define	LOOP_ACTIVE	LOW
#define LOOP_INACTIVE HIGH

volatile bool	g_inputavail( false );
unsigned int	g_input(0xffff);
unsigned int	g_rawinput(0);
bool			g_sweep(false);
unsigned char	g_sendcount(0);
unsigned char	g_remainingsends(0);

#define ST_WHITE	HIGH
#define ST_BLACK	LOW
//#define ST_WHITE	LOW
//#define ST_BLACK	HIGH

void isr();
void processinput();
void printcodes();

//////////////////////////////////////////////////////////////////////////////
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

	Serial.begin(BAUD);
}

//////////////////////////////////////////////////////////////////////////////
void loop()
{
	if( getlinefromserial( g_iobuf, sizeof( g_iobuf ), g_inidx) )
		processinput();

	if(g_remainingsends)
	{
		if( !g_inputavail ) {
			g_rawinput = g_input;// << 2;
			g_inputavail = true;

			printcodes();

			if(g_remainingsends != 0xff )
				--g_remainingsends;
		}
	}
	else if(g_sweep && !g_inputavail)
	{
		if(!g_remainingsends) {
			++g_input;
			g_remainingsends = g_sendcount;
		} else {
			--g_remainingsends;
		}

		printcodes();

		for( int retry = 0; retry < 4; ++retry ) {
			g_rawinput = g_input; //<< 2
			g_inputavail = true;
		}
	}
}


//////////////////////////////////////////////////////////////////////////////
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
			data = g_rawinput;
			g_inputavail = false;
			curbit = bitphase = 0;
			bitmask = 1;
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
			bitmask <<= 1;
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


//////////////////////////////////////////////////////////////////////////////
void processinput()
{
	const char	*inptr( g_iobuf );

	Serial.print(CMNT);
	Serial.println( g_iobuf );

	if( iscommand( inptr, F("stop"))) {
		g_sweep = false;
		g_sendcount = 0;
		g_remainingsends = 0;
		Serial.println(F(CMNTS "OK"));

	} else if(iscommand(inptr, F("send"))) {
		g_input = getintparam(inptr);
		g_remainingsends = getintparam(inptr);

	} else if(iscommand(inptr, F("sweep"))) {
		g_sendcount = getintparam(inptr);
		if(g_sendcount == 0xff)
			g_sendcount = 4;
		g_remainingsends = g_sendcount;

		g_input = getintparam(inptr);
		if(g_input == 0xffff)
			g_input = 0;
		g_sweep = true;
	} else if(iscommand(inptr, F("tone"))) {
		uint16_t	freq = getintparam((inptr));
		int 		pin = getintparam(inptr);
		if(freq == (uint16_t)-1) freq = 1000;
		if(pin == (uint16_t)-1) pin = 8;
		pinMode(pin, OUTPUT);
		tone(pin, freq);
	} else if(iscommand(inptr, F("notone"))) {
		int 		pin = getintparam(inptr);
		if(pin == (uint16_t)-1) pin = 8;
		noTone(pin);
	} else {
		Serial.println( F(ERRS "CMD"));
	}
	g_inidx = 0;
}

//////////////////////////////////////////////////////////////////////////////
void printcodes()
{
	Serial.print(F(CMNTS "Sending code "));
	Serial.print(g_input);
	Serial.print(F(" -> "));
	Serial.println(g_remainingsends);
}
