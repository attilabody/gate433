#define BAUDRATE 57600
#define ITEMCOUNT(A) (sizeof(A)/sizeof(A[0]))
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
		if( ! g_codeready && lastlevel && !in && deltat > 390 && deltat < 500) {	// h->l
			state = DATA;
			curbit = 0;
			code = 0;
		} else
			++g_stats.startabort;

		break;

	case DATA:
		if( deltat < 390 || deltat > 1000 ) {
			state = START;
			++g_stats.dataabort;
		} else if( !in ) { 	// h->l
			highdeltat = deltat;
			cyclet = highdeltat + lowdeltat;
			timediff = (int)highdeltat - (int)lowdeltat;
			if (timediff < 0) timediff = -timediff;
			if (cyclet < 1200 || cyclet > 1500 || (unsigned int)timediff < (cyclet >> 2)) {
				state = START;
				++g_stats.dataabort;
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
		if( in && deltat > 15000) {		// l->h => stop end
			if( g_codeready ) g_overrun = true;
			g_code = code;
			g_codeready = true;
			g_codetime = lastedge;
		} else {
			++g_stats.stopabort;
			g_stats.stopdeltat = deltat;
		}

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

//The setup function is called once at startup of the sketch
void setup()
{
// Add your initialization code here
	pinMode(g_ledPin, OUTPUT);
	pinMode(g_inPin, INPUT);

	noInterrupts();           // disable all interrupts
	TIMSK0 |= (1 << OCIE0A);  // enable timer compare interrupt
	interrupts();             // enable all interrupts

	Serial.begin(BAUDRATE);
	memset( (void*) &g_stats, sizeof( g_stats ), 0 );
	attachInterrupt(digitalPinToInterrupt(g_inPin), isr, CHANGE);
}

// The loop function is called in an endless loop
void loop()
{
	static unsigned int 	code, prevcode(-1);
	static unsigned long	prevcodetime(0);
	static stats			prevstats;
	static stats			*pp, *ps;

	if( g_codeready )
	{
		code = g_code;
		g_codeready = false;
		if( code != prevcode || g_codetime - prevcodetime > 1000000 ) {
			String	s(String( "ID " ) +
					String( code >>2, DEC ) +
					String( " / " ) +
					String( code & 3, DEC)
			);
			Serial.println( s );
		}
	}
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
}
