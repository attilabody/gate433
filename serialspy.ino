#define NUMDELTAS 768
#define FREQ 9600
#define BAUDRATE 57600
#define LEDDIVIDER 1000

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
    bool state = val & ((unsigned long)1 << (MSB - 1));
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
  
  if ( ++isrCounter >= LEDDIVIDER )
  {
    isrCounter = 0;
    ledStatus = !ledStatus;
    digitalWrite( ledPin, ledStatus ? HIGH : LOW );
  }
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
  Serial.begin(BAUDRATE);
  attachInterrupt(digitalPinToInterrupt(inPin), isr, CHANGE);
  tone( outPin, FREQ );
  Serial.println( "Start" );
  micro = micros();
}


