#define USE_DS3231
//#define FAILSTATS
//#define VERBOSE
//#define DBGSERIALIN
//#define EXPECT_RESPONSE

#include "gatelogic.h"
#include <Wire.h>
#include <ds3231.h>
#include "../interface/interface.h"

#define ITEMCOUNT(A) (sizeof(A)/sizeof(A[0]))

#define SHORT_MIN_TIME	340
#define SHORT_MAX_TIME	510
#define LONG_MIN_TIME	650
#define LONG_MAX_TIME	1100
#define CYCLE_MAX_TIME	( SHORT_MAX_TIME + LONG_MAX_TIME )
#define CYCLE_MIN_TIME	( SHORT_MIN_TIME + LONG_MIN_TIME )
#define	STOP_MIN_TIME	13000

const uint8_t g_inPin( 2 );
const uint8_t g_ledPin( 13 );

enum RcvState : uint8_t {
	  START
	, DATA
	, STOP
};

volatile bool g_codeready( false );
volatile unsigned int g_code;
volatile unsigned long g_codetime( 0 );

volatile unsigned long g_lastedge;

#ifdef FAILSTATS
struct stats
{
	stats() {startabort = dataabort = stopabort = stopdeltat = 0;}
	bool operator==( const stats &o ) {
		return startabort == o.startabort && dataabort == o.dataabort && stopabort == o.stopabort;
	}
	bool operator==( stats &o ) {
		return startabort == o.startabort && dataabort == o.dataabort && stopabort == o.stopabort;
	}
	stats& operator=( const stats &o ) {
		startabort = o.startabort; dataabort = o.dataabort; stopabort = o.stopabort; return *this;
	}
	unsigned long startabort, dataabort, stopabort, stopdeltat;
};

volatile stats g_stats;
#endif	//	FAILSTATS

char g_inbuf[32];
unsigned char g_inidx( 0 );
const char * g_commands[] = {
		  "gdt"
		, "sdt"
		, ""
};

void isr();
void processInput();
#ifdef USE_DS3231
void datetimetoserial( const ts &t );
#endif	//	USE_DS3231

//////////////////////////////////////////////////////////////////////////////
void setup()
{
	Serial.begin( BAUDRATE );
#ifdef USE_DS3231
#ifdef VERBOSE
	delay(100);
	Serial.println("Initializing DS3231");
#endif
	Wire.begin();
	DS3231_init( DS3231_INTCN );
#ifdef VERBOSE
	Serial.println("DS3231");
#endif
#endif	//	USE_DS3231

	pinMode( g_ledPin, OUTPUT );
	pinMode( g_inPin, INPUT );

	noInterrupts();
	// disable all interrupts
	TIMSK0 |= ( 1 << OCIE0A );  // enable timer compare interrupt
	interrupts();
	// enable all interrupts

#if defined(VERBOSE) && defined(USE_DS3231)
	ts t;
	DS3231_get( &t );
	datetimetoserial( t );
#endif	//	defined(VERBOSE) && defined(USE_DS3231)
#ifdef FAILSTATS
	memset( (void*) &g_stats, sizeof( g_stats ), 0 );
#endif

	attachInterrupt( digitalPinToInterrupt( g_inPin ), isr, CHANGE );
}

//////////////////////////////////////////////////////////////////////////////
void loop()
{
	static bool	flipflop(false);
#ifdef USE_DS3231
	static ts t;
#endif	//	USE_DS3231

#ifdef FAILSTATS
	static stats prevstats;
	static stats *pp, *ps;
#endif

	if( g_codeready ) {
#ifdef VERBOSE
		Serial.print( "ID " );
		Serial.print( g_code >>2 );
		Serial.print( " / " );
		Serial.print( g_code & 3 );
		Serial.print( " - " );
		Serial.print( " ");
#ifdef USE_DS3231
		DS3231_get( &t );
		datetimetoserial( t );
#endif	//	USE_DS3231
		Serial.println();
#else
		Serial.print( "CODE " );
		Serial.println( g_code >> 2, DEC );
#ifdef EXPECT_RESPONSE
		while( !getlinefromserial());
		Serial.println( (int) g_inidx);
#else
		strcpy( g_inbuf, flipflop ? ":000 59F 000 59F 000007F" : ":1E0 455 1E0 455 000001F");
		flipflop = ! flipflop;
		g_inidx = strlen( g_inbuf ) + 1;
#endif	//	EXPECT_RESPONSE
		//process received info here
		g_inidx = 0;
		g_codeready = false;
#endif	//	VERBOSE
	}
#ifdef FAILSTATS
	else
	{
		ps = (stats*)&g_stats;
		if( !(prevstats == *ps) )
		{
			String s( String( g_stats.startabort )
					+ String( " " ) + String( g_stats.dataabort )
					+ String( " " ) + String( g_stats.stopabort )
					+ String( " " ) + String( g_stats.stopdeltat )
			);
			Serial.println( s );
			prevstats = *ps;
		}
	}
#endif	//	FAILSTATS

	if( getlinefromserial( g_inbuf, sizeof(g_inbuf ), g_inidx) )
		processInput();
}

//////////////////////////////////////////////////////////////////////////////
void isr()
{
	static int8_t curbit;
	static uint32_t lastedge( micros() ), curedge;
	static bool lastlevel( digitalRead( g_inPin ) == HIGH ), in;
	static RcvState state( START );
	static uint16_t code, deltat, cyclet;
	static int timediff;
	static uint16_t prevcode( 0 );
	static uint32_t prevcodetime( 0 );

	static uint32_t highdeltat, lowdeltat;

	curedge = micros();
	in = ( digitalRead( g_inPin ) == HIGH );
	deltat = curedge - lastedge;

	switch( state ) {
	case START:
		if( !g_codeready && lastlevel && !in && deltat >= SHORT_MIN_TIME
		        && deltat <= SHORT_MAX_TIME ) {	// h->l
			state = DATA;
			curbit = code = 0;
		}
#ifdef FAILSTATS
		else
		++g_stats.startabort;
#endif

		break;

	case DATA:
		if( deltat < SHORT_MIN_TIME || deltat > LONG_MAX_TIME ) {
			state = START;
#ifdef FAILSTATS
			++g_stats.dataabort;
#endif
		} else if( !in ) { 	// h->l
			highdeltat = deltat;
			cyclet = highdeltat + lowdeltat;
			timediff = (int)highdeltat - (int)lowdeltat;
			if( timediff < 0 )
				timediff = -timediff;
			if( cyclet < CYCLE_MIN_TIME || cyclet > CYCLE_MAX_TIME
			        || (unsigned int)timediff < ( cyclet >> 2 ) ) {
				state = START;
#ifdef FAILSTATS
				++g_stats.dataabort;
#endif
				break;
			}
			code <<= 1;
			if( lowdeltat < highdeltat )
				code |= 1;
			if( ++curbit == 12 )
				state = STOP;
		} else {			// h -> l
			lowdeltat = deltat;
		}
		break;

	case STOP:
		if( in && deltat > STOP_MIN_TIME && ( !g_codeready )
		        && ( ( code != g_code ) || ( lastedge - g_codetime > 500000 ) ) ) {	// l->h => stop end
			g_code = code;
			g_codeready = true;
			g_codetime = lastedge;
		}
#ifdef FAILSTATS
		else {
			++g_stats.stopabort;
			g_stats.stopdeltat = deltat;
		}
#endif
		state = START;
		break;
	}

	lastlevel = in;
	g_lastedge = lastedge = curedge;
}

//////////////////////////////////////////////////////////////////////////////
ISR( TIMER0_COMPA_vect ) {
	digitalWrite( g_ledPin, ( micros() - g_codetime < 500000 ) ? HIGH : LOW );
}

//////////////////////////////////////////////////////////////////////////////
inline void halfbytetohex( unsigned char data, char* &buffer ) {
	*buffer++ = data + ( data < 10 ? '0' : ( 'A' - 10 ) );
}

//////////////////////////////////////////////////////////////////////////////
inline void bytetohex( unsigned char data, char* &buffer, bool both ) {
	if( both )
		halfbytetohex( data >> 4, buffer );
	halfbytetohex( data & 0x0f, buffer );
}

//////////////////////////////////////////////////////////////////////////////
void uitohex( uint16_t data, char* &buffer, uint16_t digits ) {
	if( digits > 2 )
		bytetohex( (unsigned char)( data >> 8 ), buffer, digits >= 4 );
	bytetohex( (unsigned char)data, buffer, digits != 1 );
}

//////////////////////////////////////////////////////////////////////////////
void ultohex( unsigned long data, char* &buffer, uint16_t digits ) {
	if( digits > 4 ) {
		uitohex( (uint16_t)( data >> 16 ), buffer, digits - 4 );
		digits -= digits - 4;
	}
	uitohex( (uint16_t)data, buffer, digits );
}

#ifdef USE_DS3231
//////////////////////////////////////////////////////////////////////////////
void serializedatetime( const ts &t, char *buffer )
{
	bytetohex( (byte)( t.year - 2000 ), buffer, true );
	bytetohex( (byte)t.mon, buffer, false );
	bytetohex( (byte)t.mday, buffer, true );
	bytetohex( (byte)t.wday, buffer, false );
	bytetohex( (byte)t.hour, buffer, true );
	bytetohex( (byte)t.min, buffer, true );
	bytetohex( (byte)t.sec, buffer, true );
	*buffer++ = 0;
}

//////////////////////////////////////////////////////////////////////////////
void datetimetoserial( const ts &t )
{
	Serial.print( t.year );
	Serial.print( '.' );
	Serial.print( t.mon );
	Serial.print( '.' );
	Serial.print( t.mday );
	Serial.print( '/' );
	Serial.print( t.wday );
	Serial.print( '-' );
	Serial.print( t.hour );
	Serial.print( ':' );
	Serial.print( t.min );
	Serial.print( ':' );
	Serial.print( t.sec );
}

//////////////////////////////////////////////////////////////////////////////
bool parsedatetime( ts &t, const char *inptr )
{
	//	"2015.10.28-3 16:37:05"
	t.year = getintparam( inptr );
	if( t.year == -1 ) return false;
	t.mon = getintparam( inptr );
	if( t.mon == -1 ) return false;
	t.mday = getintparam( inptr );
	if( t.mday == -1 ) return false;
	t.wday = getintparam( inptr );
	if( t.wday == -1 ) return false;
	t.hour = getintparam( inptr );
	if( t.hour == -1 ) return false;
	t.min = getintparam( inptr );
	if( t.min == -1 ) return false;
	t.sec = getintparam( inptr );
	if( t.sec == -1 ) t.sec = 0;
	return true;
}

#endif	//	USE_DS3231

//////////////////////////////////////////////////////////////////////////////
void processInput()
{
	static char dtbuffer[13];

	const char *inptr( g_inbuf );
	int param( 0 );

	char command = findcommand( inptr, g_commands );
#ifdef VERBOSE
	Serial.print( CMNT );
	Serial.println( (int)command );
#endif

	switch( command ) {
	default:
		Serial.print( ERR "Error (command) " );
		Serial.println( g_inbuf );
		break;
	case 0:		//	gdt
#ifdef USE_DS3231
	{
		ts t;
		DS3231_get( &t );
		serializedatetime( t, dtbuffer );
		Serial.print( RESP );
		Serial.print( dtbuffer );
		Serial.print( ' ' );
		datetimetoserial( t );
		Serial.println();
	}
#else	//	USE_DS3231
		Serial.println( ERR "Not implemented" );
#endif	//	USE_DS3231
		break;

	case 1:		//sdt
#ifdef USE_DS3231
	{
		ts t;
		if( parsedatetime( t, inptr )) {
			DS3231_set( t );
			Serial.println( RESP "OK" );
		} else {
			Serial.println( ERR "Datetime" );
		}

	}
#else	//	USE_DS3231
#endif	//	USE_DS3231
		break;
	}
	g_inidx = 0;
}
