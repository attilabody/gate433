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
		if( ! g_codeready && lastlevel && !in && deltat > 420 && deltat < 450) {	// h->l
			state = DATA;
			curbit = 0;
			code = 0;
		}
		break;

	case DATA:
		if( deltat < 400 || deltat > 1000 ) {
			state = START;
		} else if( !in ) { 	// h->l
			highdeltat = deltat;
			cyclet = highdeltat + lowdeltat;
			timediff = (int)highdeltat - (int)lowdeltat;
			if (timediff < 0) timediff = -timediff;
			if (cyclet < 1200 || cyclet > 1500 || (unsigned int)timediff < (cyclet >> 2)) {
				state = START;
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
	attachInterrupt(digitalPinToInterrupt(g_inPin), isr, CHANGE);
}

// The loop function is called in an endless loop
void loop()
{
	static unsigned int 	code, prevcode(-1);
	static unsigned long	prevcodetime(0);
	if( g_codeready )
	{
		code = g_code;
		g_codeready = false;
		if( code != prevcode || g_codetime - prevcodetime > 1000000 ) {
			String	s("ID " + String( code >>2, DEC ) + " / " + String( code & 3, DEC));
			Serial.println( s );
		}
	}
}
