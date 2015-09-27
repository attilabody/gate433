#define NUMDELTAS 512
#define FREQ 10

unsigned long iter(0);
bool          state;
unsigned long micro;


unsigned int deltas[NUMDELTAS];
unsigned int  cursample(0);
typedef unsigned int datatype;
#define MSB ( sizeof( datatype ) * 8 )

const int outPin = 12;
const int inPin = 2;
const int ledPin = 13;
const int analogInPin = A0;  // Analog input pin that the potentiometer is attached to

int sv, asv;
bool  first = true;

void printDeltas()
{
  unsigned long start( micros() );
  for( int sample=0; sample<NUMDELTAS; ++sample )
  {
    unsigned long val( deltas[sample] );
    bool state = val & ((unsigned long)1 << (MSB-1));
    val &= ((datatype)(-1)) >> 1;
    Serial.print( state ? "HL " : "LH " );
    Serial.println( val );
  }
  Serial.print("----- Communication took ");
  Serial.print( micros() - start );
  Serial.println( " microseconds." );
  Serial.println("");
}

/*void loop()
{
  if( state != (digitalRead( inPin ) == HIGH))
  {
    state = !state;
    digitalWrite( ledPin, state ? HIGH : LOW );
    Serial.print( iter++ );
    Serial.print(": ");
    Serial.println( state );
  }
}
*/
void loop()
{
  if( state != (digitalRead( inPin ) == HIGH))
  {
    unsigned long curmicro = micros();
    unsigned long delta = curmicro - micro;
    if( state ) {
      delta |= (((unsigned long)1) << (MSB-1));
    }
    
    deltas[cursample++] = delta;
    if( cursample >= NUMDELTAS )
    {
      printDeltas();
      curmicro = micros();
      cursample=0;
      state = (digitalRead( inPin ) == LOW); //we will invert it
    }
    micro = curmicro;
    state = !state;
    digitalWrite( ledPin, state ? HIGH : LOW );
  }
}

void setup()
{
  pinMode(outPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(inPin, INPUT);
  analogReference( DEFAULT );
  Serial.begin(115200);
  tone( outPin, FREQ );
  Serial.println( "Start" );
  state = (digitalRead( inPin ) == HIGH);
  micro = micros();
}


