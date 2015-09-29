#define NUMDELTAS 768
#define FREQ 1000
#define BAUDRATE 57600
#define LEDDIVIDER 100

typedef unsigned int datatype;


unsigned long iter(0);
unsigned long micro(0), curmicro;


volatile unsigned int deltas[NUMDELTAS];
volatile unsigned int cursample(0);

#define MSB ( sizeof( datatype ) * 8 )

const int outPin = 12;
const int inPin = 2;
const int ledPin = 13;

volatile bool   full = false;

void printDeltas()
{
  bool          state;
  unsigned long start( micros() );

  for ( int sample = 0; sample < NUMDELTAS; ++sample )
  {
    unsigned long val( deltas[sample] );
    bool state = val & ((datatype)1 << (MSB - 1));
    val &= ((datatype)(-1)) >> 1;
    Serial.print( state ? "HL " : "LH " );
    Serial.println( val );
  }
  Serial.print("----- Communication took ");
  Serial.print( micros() - start );
  Serial.println( " microseconds." );
  Serial.println("");
}


bool          ledStatus(false);
unsigned int  isrCounter(0);
bool          in;

void isr()
{
  curmicro = micros();
  
  if( full ) {
    micro = curmicro;
    return;
  }

  in = (digitalRead(inPin) == HIGH);
  deltas[cursample++] = (curmicro - micro) | (((unsigned long)in) << (MSB-1));
  if( cursample == NUMDELTAS ) {
    full=true;
  }
  micro = curmicro;
}

void loop()
{
  while( !full );
  printDeltas();
  cursample = 0;
  full = false;
}

void setup()
{
  pinMode(outPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(inPin, INPUT);
  analogReference( DEFAULT );

  noInterrupts();           // disable all interrupts
  TIMSK0 |= (1 << OCIE0A);  // enable timer compare interrupt
  interrupts();             // enable all interrupts

  Serial.begin(BAUDRATE);
  attachInterrupt(digitalPinToInterrupt(inPin), isr, CHANGE);
  tone( outPin, FREQ );
  Serial.println( "Start" );
  micro = micros();
}

ISR( TIMER0_COMPA_vect )
{
  digitalWrite( ledPin, ( !full && micros()-micro < 100000 ) ? HIGH : LOW );
}

