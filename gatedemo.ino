#define BAUDRATE 57600
#define ITEMCOUNT(A) (sizeof(A)/sizeof(A[0]))
const uint8_t g_inPin( 2 );
const uint8_t g_ledPin( 13 );

enum RcvState {
	  START = 0
	, DATA
	, STOP
};

volatile unsigned int	g_inbuf;
volatile bool			g_inputready(false);
volatile unsigned long	g_lastedge;

void isr()
{
	static unsigned char	curbit;
	static unsigned long	lastedge( micros()), curedge;
	static bool				lastlevel(digitalRead(g_inPin) == HIGH), in;
	static RcvState			state( START );
	static unsigned int		deltat, cyclet;
	static int				timediff;

	static unsigned long	lowdeltat, highdeltat;

	curedge = micros();
	in = (digitalRead(g_inPin) == HIGH);
	deltat = curedge - lastedge;

	if( ! g_inputready )
	{
		switch( state )
		{
		case START:
			if( !lastlevel && in && deltat > 420 && deltat < 450) {	//high to low
				state = DATA;
				curbit = 0;
				g_inbuf = 0;
			}
			break;

		case DATA:
			if( deltat < 400 || deltat > 1000 ) {
				state = START;
			} else if( in ) { 	// l->h
				lowdeltat = deltat;
				cyclet = lowdeltat + highdeltat;
				timediff = (int)lowdeltat - (int)highdeltat;
				if (timediff < 0) timediff = -timediff;
				if (cyclet < 1200 || cyclet > 1500 || (unsigned int)timediff < (cyclet >> 2)) {
					state = START;
					break;
				}
				g_inbuf <<= 1;
				if (highdeltat < lowdeltat)
					g_inbuf |= 1;
				if (++curbit == 12)
					state = STOP;
			} else {			// h -> l
				highdeltat = deltat;
			}
			break;

		case STOP:
			if( !in && deltat > 15000) {		// high to low -> stop end
				g_inputready = true;
			}
			state = START;
			break;
		}
	}

	lastlevel = in;
	g_lastedge = lastedge = curedge;
}

ISR( TIMER0_COMPA_vect )
{
	static unsigned long now;

	now = micros();
	digitalWrite( g_ledPin, ( now - g_lastedge < 100000 ) ? HIGH : LOW );
}

//The setup function is called once at startup of the sketch
void setup()
{
// Add your initialization code here
	pinMode(g_ledPin, OUTPUT);
	pinMode(g_inPin, INPUT);
	Serial.begin(BAUDRATE);
	attachInterrupt(digitalPinToInterrupt(g_inPin), isr, CHANGE);
}

// The loop function is called in an endless loop
void loop()
{
	static unsigned int code;
	if( g_inputready )
	{
		code = g_inbuf;
		g_inputready = false;
		Serial.println( code );
	}
	else
	{
		//Serial.println( (unsigned int)(micros - g_lastedge) );
	}
}
