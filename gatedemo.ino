#include <ds3231/ds3231.h>

#define BAUDRATE 57600
#define ITEMCOUNT(A) (sizeof(A)/sizeof(A[0]))

#define SHORT_MIN_TIME	340
#define SHORT_MAX_TIME	510
#define LONG_MIN_TIME	650
#define LONG_MAX_TIME	1100
#define CYCLE_MAX_TIME	( SHORT_MAX_TIME + LONG_MAX_TIME )
#define CYCLE_MIN_TIME	( SHORT_MIN_TIME + LONG_MIN_TIME )
#define	STOP_MIN_TIME	13000

//#define FAILSTATS
//#define VERBOSE

const uint8_t g_inPin( 2 );
const uint8_t g_ledPin( 13 );

enum RcvState {
	  START = 0
	, DATA
	, STOP
};

volatile bool			g_codeready(false), g_overrun(false);
volatile unsigned int	g_code;
volatile unsigned long	g_codetime(0);
volatile unsigned long	g_lastedge;

#ifdef FAILSTATS
struct stats
{
	bool operator==( const stats &o ) {
		return startabort == o.startabort && dataabort == o.dataabort && stopabort == o.stopabort;
	}
	bool operator==( stats &o ) {
		return startabort == o.startabort && dataabort == o.dataabort && stopabort == o.stopabort;
	}
	stats& operator=( const stats &o ) {
		startabort = o.startabort; dataabort = o.dataabort; stopabort = o.stopabort; return *this;
	}
	unsigned long	startabort, dataabort, stopabort, stopdeltat;
};

volatile stats	g_stats;
#endif	//	FAILSTATS

char g_serbuf[32];
unsigned char g_serptr(0);
const char * g_commands[] = {
  "settime"
};


void isr();
void processInput();

void setup()
{
// Add your initialization code here
	pinMode(g_ledPin, OUTPUT);
	pinMode(g_inPin, INPUT);

	noInterrupts();           // disable all interrupts
	TIMSK0 |= (1 << OCIE0A);  // enable timer compare interrupt
	interrupts();             // enable all interrupts
    DS3231_init(DS3231_INTCN);

	Serial.begin(BAUDRATE);
#ifdef FAILSTATS
	memset( (void*) &g_stats, sizeof( g_stats ), 0 );
#endif
	attachInterrupt(digitalPinToInterrupt(g_inPin), isr, CHANGE);
}

void isr()
{
	static unsigned char	curbit;
	static unsigned long	lastedge( micros()), curedge;
	static bool				lastlevel(digitalRead(g_inPin) == HIGH), in;
	static RcvState			state( START );
	static unsigned int		code, deltat, cyclet;
	static int				timediff;

	static unsigned long	highdeltat, lowdeltat;

	curedge = micros();
	in = (digitalRead(g_inPin) == HIGH);
	deltat = curedge - lastedge;

	switch( state )
	{
	case START:
		if( ! g_codeready && lastlevel && !in && deltat >= SHORT_MIN_TIME && deltat <= SHORT_MAX_TIME) {	// h->l
			state = DATA;
			curbit = 0;
			code = 0;
		} else
#ifdef FAILSTATS
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
			if (timediff < 0) timediff = -timediff;
			if (cyclet < CYCLE_MIN_TIME || cyclet > CYCLE_MAX_TIME || (unsigned int)timediff < (cyclet >> 2)) {
				state = START;
#ifdef FAILSTATS
				++g_stats.dataabort;
#endif
				break;
			}
			code <<= 1;
			if (lowdeltat < highdeltat)
				code |= 1;
			if (++curbit == 12)
				state = STOP;
		} else {			// h -> l
			lowdeltat = deltat;
		}
		break;

	case STOP:
		if( in && deltat > STOP_MIN_TIME) {		// l->h => stop end
			if( g_codeready ) g_overrun = true;
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

ISR( TIMER0_COMPA_vect )
{
	digitalWrite( g_ledPin, ( micros() - g_codetime  < 500000 ) ? HIGH : LOW );
}

// The loop function is called in an endless loop
void loop()
{
	static unsigned int 	code, prevcode(-1);
	static unsigned long	prevcodetime(0);
	static unsigned long	cdt;
#ifdef FAILSTATS
	static stats			prevstats;
	static stats			*pp, *ps;
#endif

	if( g_codeready )
	{
		code = g_code;
		g_codeready = false;

		cdt = g_codetime - prevcodetime;

		if( code != prevcode || cdt > 1000000 )
		{
			prevcode = code;
			prevcodetime = g_codetime;
#ifndef VERBOSE
			String	s( code >> 2 );
#else
			String	s(String( "ID " ));
			s.concat( code >>2 );
			s.concat( " / " );
			s.concat( code & 3 );
			s.concat( " - " );
			s.concat( cdt );
#endif	//	VERBOSE

			Serial.println( s );
		}
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
}

char findcommand(unsigned char &inptr)
{
	while (inptr < g_serptr && g_serbuf[inptr] != ' ' && g_serbuf[inptr] != ','
			&& g_serbuf[inptr] != '\n')
		++inptr;

	if (inptr == g_serptr) return -1;

	for (char i = 0; i < ITEMCOUNT(g_commands); ++i)
	{
		if (!strncmp(g_serbuf, g_commands[i], inptr))
		{
			++inptr;
			while (inptr < g_serptr
					&& (g_serbuf[inptr] == ' ' || g_serbuf[inptr] == '\n')
					|| g_serbuf[inptr] == ',')
				++inptr;
			return i;
		}
	}
	return -1;
}

int getintparam(unsigned char &inptr)
{
	int retval(0);
	bool found(false);
	while (inptr < g_serptr && isdigit(g_serbuf[inptr]))
	{
		retval *= 10;
		retval += g_serbuf[inptr++] - '0';
		found = true;
	}

	while (inptr < g_serptr
			&& (g_serbuf[inptr] == ' ' || g_serbuf[inptr] == '\n')
			|| g_serbuf[inptr] == ',')
		++inptr;

	return found ? retval : -1;
}

void processInput()
{
	unsigned char inptr(0);
	int param(0);

	while (Serial.available())
	{
		char inc = Serial.read();
		g_serbuf[g_serptr++] = inc;
		if (inc == '\n' || g_serptr == sizeof(g_serbuf)) {
			processInput();
			g_serptr = 0;
		}
	}
	char command = findcommand(inptr);

	switch (command) {
	case 0:		//settime
		break;
	}
}

