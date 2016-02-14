#define NUMDELTAS 600
#define BAUDRATE 115200
#define MSB ( sizeof( datatype ) * 8 )

#define ITEMCOUNT(A) (sizeof(A)/sizeof(A[0]))

typedef unsigned int datatype;

unsigned long g_iter(0);
unsigned long g_lastedge;

volatile unsigned int g_deltas[NUMDELTAS];
volatile unsigned int g_cursample(0);

const int g_outPin( 12 );
const int g_inPin( 2 );
const int g_ledPin( 13 );

volatile bool   g_full( false );

char g_serbuf[32];
unsigned char g_serptr(0);

const char * g_commands[] = {
  "tone"
};



//////////////////////////////////////////////////////////////////////////////
void printDeltas()
{
  bool          state;
  unsigned long start( micros() );

  for ( int sample = 0; sample < g_cursample; ++sample )
  {
    unsigned long val( g_deltas[sample] );
    bool state = val & ((datatype)1 << (MSB - 1));
    val &= ((datatype)(-1)) >> 1;
    if( sample && val > 10000 ) Serial.println();
    else if( sample ) Serial.print('\t');
//    Serial.print( state ? "L " : "H " );	//high after change means pulse was low
    Serial.print( val );
  }
  Serial.println();
  Serial.print("----- Communication took ");
  Serial.print( micros() - start );
  Serial.println( " microseconds." );
  Serial.println("");
}

//////////////////////////////////////////////////////////////////////////////
void isr()
{
  static unsigned long  now;
  static bool           in;
  static unsigned int	delta;

  now = micros();

  if ( g_full ) {
    g_lastedge = now;
    return;
  }
  in = (PIND & 4) != 0;	//digitalRead(2) == HIGH;
  delta = (now - g_lastedge);

  if( !g_cursample )
	  if ( !in || delta < 10000 ) {
	    g_lastedge = now;
	    return;
  }

  g_deltas[g_cursample++] = delta | (((unsigned long)in) << (MSB - 1));
  if ( g_cursample == NUMDELTAS ) {
    g_full = true;
  }
  g_lastedge = now;
}

//////////////////////////////////////////////////////////////////////////////
char findcommand(unsigned char &inptr)
{
  while( inptr < g_serptr && g_serbuf[inptr] != ' ' && g_serbuf[inptr] != ',' && g_serbuf[inptr] != '\n' ) {
    ++inptr;
  }
  
  if (inptr == g_serptr) return -1;

  for (char i = 0; i < ITEMCOUNT(g_commands); ++i) {
    if (!strncmp(g_serbuf, g_commands[i], inptr)) {
      ++inptr;
      while( inptr < g_serptr && (g_serbuf[inptr] == ' ' || g_serbuf[inptr] == '\n') || g_serbuf[inptr] == ',' ) ++inptr;
      return i;
    }
  }
  return -1;
}

//////////////////////////////////////////////////////////////////////////////
int getintparam(unsigned char &inptr)
{
  int retval(0);
  bool found(false);
  while (inptr < g_serptr && isdigit(g_serbuf[inptr])) {
    retval *= 10;
    retval += g_serbuf[inptr++] - '0';
    found = true;
  }
  while( inptr < g_serptr && (g_serbuf[inptr] == ' ' || g_serbuf[inptr] == '\n') || g_serbuf[inptr] == ',' ) ++inptr;

  return found ? retval : -1;
}

//////////////////////////////////////////////////////////////////////////////
void processInput()
{
  unsigned char inptr(0);
  int param(0);
  char command = findcommand(inptr);

  switch (command)
  {
    case 0:
      param = getintparam( inptr );
      if (param == -1) noTone( g_outPin );
      else tone( g_outPin, param );
  }
}

//////////////////////////////////////////////////////////////////////////////
void loop()
{
  if ( g_full ) {
    printDeltas();
    g_cursample = 0;
    g_full = false;
  }
  while ( Serial.available() ) {
    char inc = Serial.read();
    g_serbuf[g_serptr++] = inc;
    if ( inc == '\n' || g_serptr == sizeof(g_serbuf) ) {
      processInput();
      g_serptr = 0;
    }
  }
}

//////////////////////////////////////////////////////////////////////////////
void setup()
{
  pinMode(g_outPin, OUTPUT);
  pinMode(g_ledPin, OUTPUT);
  pinMode(g_inPin, INPUT);
  analogReference( DEFAULT );

  noInterrupts();           // disable all interrupts
  TIMSK0 |= (1 << OCIE0A);  // enable timer compare interrupt
  interrupts();             // enable all interrupts

  Serial.begin(BAUDRATE);
  attachInterrupt(digitalPinToInterrupt(g_inPin), isr, CHANGE);
  Serial.println( ">>> Start <<<" );
  //g_serbuf[sizeof(g_serbuf) - 1] = 0;
  g_lastedge = micros();
}

ISR( TIMER0_COMPA_vect )
{
  unsigned long now;

  now = micros();
  digitalWrite( g_ledPin, ( !g_full && now - g_lastedge < 100000 ) ? HIGH : LOW );
  if ( g_cursample && !g_full && now - g_lastedge > 1000000 )
    g_full = true;
}

