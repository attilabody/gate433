// Do not remove the include below
#include "Arduino.h"
#include "interface.h"
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library
#include <TouchScreen.h>
#include <SPI.h>
#include <SD.h>

#define ITEMCOUNT(A) (sizeof(A)/sizeof(A[0]))

// The control pins for the LCD can be assigned to any digital or
// analog pins...but we'll use the analog pins as this allows us to
// double up the pins with the touch screen (see the TFT paint example).
// #define LCD_CS A3 // Chip Select goes to Analog 3
// #define LCD_CD A2 // Command/Data goes to Analog 2
// #define LCD_WR A1 // LCD Write goes to Analog 1
// #define LCD_RD A0 // LCD Read goes to Analog 0

// #define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

// When using the BREAKOUT BOARD only, use these 8 data lines to the LCD:
// For the Arduino Uno, Duemilanove, Diecimila, etc.:
//   D0 connects to digital pin 8  (Notice these are
//   D1 connects to digital pin 9   NOT in order!)
//   D2 connects to digital pin 2
//   D3 connects to digital pin 3
//   D4 connects to digital pin 4
//   D5 connects to digital pin 5
//   D6 connects to digital pin 6
//   D7 connects to digital pin 7
// For the Arduino Mega, use digital pins 22 through 29
// (on the 2-row header at the end of the board).

// Assign human-readable names to some common 16-bit color values:
#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0

#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin
#define	BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF800
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define GREY	0x7BEF
#define D_GREEN	0x03E0
#define D_GREY	0x39e7

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

#define YP A3  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM 9   // can be a digital pin
#define XP 8   // can be a digital pin

#define TS_MINX 189
#define TS_MINY 174
#define TS_MAXX 900
#define TS_MAXY 950

TouchScreen	g_ts( XP, YP, XM, YM, 258 );
int16_t		g_bottom( TS_MINY ), g_left( TS_MINX ), g_top( TS_MAXY), g_right( TS_MAXX );
bool		g_owner( true );

void	printCode( int code );
void	processInput();
long	getintparam(unsigned char &sbindex, bool decimal = true );
void	drawbuttons( bool first, uint16_t color_h = D_GREEN, uint16_t color_l = D_GREY );
bool	getlinefromserial();
void	printInput();

#ifdef VERBOSE
template< typename T1, typename T2 > void dbgout( const T1 &a, const T2 &b, bool newline = true )
{
	Serial.print( a );
	Serial.print( ": " );
	Serial.print( b );
	if( newline ) Serial.println();
	else Serial.print( " - " );
}
#endif

char 			g_serbuf[256];
unsigned char	g_serptr(0);
const char 		*g_commands[] = {
	  "CODE"
	, "GET"
	, "SET"
	, "SETF"
	, "LOG"
};

uint16_t	g_buttonheight, g_buttonwidth, g_buttontop;
bool		g_recordmode( true );

//////////////////////////////////////////////////////////////////////////////
void setup(void)
{
	Serial.begin( BAUDRATE);
	tft.reset();
	tft.begin(tft.readID());
	tft.setRotation(2);
	delay(100);
	tft.fillScreen( BLACK);

	tft.setTextSize(5);
	if (!SD.begin(10)) {
		tft.setTextColor(RED);
		tft.println("SD FAIL");
	} else {
		tft.setTextColor(GREEN);
		tft.println("SD OK");
	}
	g_buttonheight = 40;
	g_buttontop = tft.height() - g_buttonheight;
	g_buttonwidth = tft.width() / 2;

	drawbuttons( true );
	drawbuttons( true );
}

//////////////////////////////////////////////////////////////////////////////
void loop(void)
{
	if( getlinefromserial()) {
		printInput();
		processInput();
	}

	TSPoint p = g_ts.getPoint();

	// if sharing pins, you'll need to fix the directions of the touchscreen pins
	//pinMode(XP, OUTPUT);
	pinMode(XM, OUTPUT);
	pinMode(YP, OUTPUT);

	if (p.z > g_ts.pressureThreshhold)
	{
		if( p.x < g_left ) g_left = p.x;
		if( p.y < g_bottom ) g_bottom = p.y;
		if( p.x > g_top ) g_top = p.x;
		if( p.y > g_right ) g_right = p.y;

		float x( (float)(p.x - g_left)/(float)(g_right - g_left) );
		float y( (float)(p.y - g_bottom)/(float)(g_top - g_bottom) );

		if( y > 0.82 )
		{
			bool owner( x < 0.5 );
			if( owner != g_owner ) {
				drawbuttons( owner);
				g_owner = owner;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
bool getlinefromserial()
{
	bool lineready(false);
	while (Serial.available() && !lineready )
	{
		char inc = Serial.read();
#if defined(DBGSERIALIN)
		g_serbuf[g_serptr ] = 0;
		Serial.print( CMNT );
		Serial.print( " " );
		Serial.println( g_serbuf );
		Serial.print( inc );
		Serial.print( ' ' );
		Serial.println( g_serptr );
#endif	//	DBGSERIALIN
		if( inc == '\n') inc = 0;
		g_serbuf[g_serptr++] = inc;
		if ( !inc || g_serptr >= sizeof( g_serbuf ) -1 )
		{
			if( inc ) g_serbuf[g_serptr] = 0;
			lineready = true;
#if defined(DBGSERIALIN)
			Serial.print( CMNT "Line ready:" );
			Serial.print( g_serbuf );
			Serial.print( "|" );
			Serial.print( (int)inc );
			Serial.print( " " );
			Serial.println( g_serptr );
			Serial.print( CMNT );
			for( char idx = 0; idx < g_serptr; ++idx ) {
				Serial.print( (int) g_serbuf[idx] );
				Serial.print( ' ' );
			}
			Serial.println();
#endif	//	DBGSERIALIN

		}
	}
	return lineready;
}

//////////////////////////////////////////////////////////////////////////////
void drawbuttons( bool first, uint16_t color_h, uint16_t color_l )
{
	tft.fillRect(0, g_buttontop, g_buttonwidth, g_buttonheight, first ? color_h : color_l );
	tft.fillRect(g_buttonwidth, g_buttontop, g_buttonwidth, g_buttonheight, first ? color_l : color_h );
	tft.setTextSize(3);
	tft.setCursor( 27, g_buttontop + 8 ); tft.setTextColor( first ? WHITE : GREY ); tft.print( "lako" );
	tft.setCursor( g_buttonwidth + 17, g_buttontop + 8); tft.setTextColor( first ? GREY : WHITE ); tft.print("berlo");
}

//////////////////////////////////////////////////////////////////////////////
void printCode( int code )
{
	tft.fillRect( 0, 0, 240, 40, BLACK );
	tft.setCursor( 0, 0 );
	tft.setTextColor( WHITE );
	tft.setTextSize( 5 );
	tft.println( code, DEC );
}

//////////////////////////////////////////////////////////////////////////////
void printInput()
{
	tft.fillRect( 0, 40, 240, 240, BLACK );
	tft.setCursor( 0, 42 );
	tft.setTextColor( WHITE );
	tft.setTextSize( 1 );
	tft.println( g_serbuf );
}

//////////////////////////////////////////////////////////////////////////////
inline char convertdigit( char c, bool decimal = true )
{
	if( decimal )
		return (c >= '0' && c <= '9') ? c - '0' : -1;
	else {
		if( c >= '0' && c <= '9' ) return c - '0';
		if( c >= 'a' && c <= 'f' ) return c - 'a' + 10;
		if( c >= 'A' && c <= 'F' ) return c - 'A' + 10;
		return -1;
	}
}

//////////////////////////////////////////////////////////////////////////////
long getintparam(unsigned char &sbidx, bool decimal)
{
	long	retval(0);
	char	converted;
	bool	found(false);


	while( sbidx < g_serptr ) {
		if(( converted = convertdigit( g_serbuf[sbidx++])) == -1) break;
		retval *=  decimal ? 10 : 16;
		retval += converted;
		found = true;
	}
	while (sbidx < g_serptr
			&& (g_serbuf[sbidx] == ' ' || g_serbuf[sbidx] == '\n'
					|| g_serbuf[sbidx] == ',')) {
		++sbidx;
	}

	return found ? retval : -1;
}

//////////////////////////////////////////////////////////////////////////////
char findcommand(unsigned char &inptr)
{
	while (inptr < g_serptr && g_serbuf[inptr] && g_serbuf[inptr] != ' ' && g_serbuf[inptr] != ','
			&& g_serbuf[inptr] != '\n')
		++inptr;
	if (inptr == g_serptr) return -1;

	for (char i = 0; i < ITEMCOUNT(g_commands); ++i)
	{
		if (!strncmp(g_serbuf, g_commands[i], inptr))
		{
			while(	inptr < g_serptr &&
					( g_serbuf[inptr] == ' ' || g_serbuf[inptr] == '\n' || g_serbuf[inptr] == ',' )
				)
				++inptr;
			return i;
		}
	}
	return -1;
}

//////////////////////////////////////////////////////////////////////////////
void processInput()
{
//	Serial.println( g_serbuf );

	static char 	linebuffer[25];
	unsigned char	inptr(0);
	char			command( findcommand(inptr) );
	int				code( 0 );

	switch (command) {
	default:
		Serial.println( ERR "Eror (command)");
		break;
	case 0:		//	CODE <CODE>
		{
			code = getintparam(inptr);

			if( code == -1 ) {
				Serial.println( ERR "Error (code)" );
				break;
			}
			bool	fail = true;
			File	file;
			if( g_recordmode )
			{
				file = SD.open( "db.txt", FILE_WRITE );
				if( !file ) {
					Serial.println( ERR "Error (open)" );
					break;
				}
				strcpy( linebuffer, g_owner ? "000 59F 000 59F 000007F\n" : "1E0 455 1E0 455 000001F\n" );
				if( !file.seek( code * RECORD_WIDTH ) )
					Serial.println( ERR "Error (seek)" );
				else if( file.write( linebuffer ) != RECORD_WIDTH )
					Serial.println( ERR "Error (file)" );
				else {
					fail = false;
				}
			}
			else
			{
				file = SD.open( "db.txt", FILE_READ );
				if( !file ) {
					Serial.println( ERR "Error (open)" );
					break;
				}
				if( !file.seek( code * RECORD_WIDTH ))
					Serial.println( ERR "Error (seek)" );
				else if( file.read( linebuffer, RECORD_WIDTH ) != RECORD_WIDTH )
					Serial.println( ERR "Error (read)" );
				else {
					linebuffer[24] = 0;
					fail = false;
				}
			}
			file.close();

			if( !fail ) {
				printCode( code );
				Serial.print( RESP );
				Serial.print( linebuffer );
			}
		}
		break;

	case 1:		//	GET <CODE>
		{
			code = getintparam(inptr);
			if( code == -1 ) {
				Serial.println( RESP "Error (code)" );
				break;
			}
			File	file( SD.open( "db.txt", FILE_READ ));
			if( !file ) {
				Serial.println( RESP "Error (open)" );
				break;
			}
			if( file.seek( code * RECORD_WIDTH )) {
				if( file.read( linebuffer, RECORD_WIDTH ) == RECORD_WIDTH ) {
					linebuffer[24] = 0;
					Serial.print( ':' );
					Serial.println( linebuffer );
				} else
					Serial.println( RESP "Error (read)" );
			} else
				Serial.println( RESP "Error (seek)" );
			file.close();
		}
		break;

	case 2:		//	SET <CODE> 000 59F 000 59F 0000000
		{
			code = getintparam(inptr);
			if( code == -1 ) {
				Serial.println( ERR "Error (code)" );
				break;
			}
			char* buf = g_serbuf + inptr;
			if( strlen( buf ) != RECORD_WIDTH - 1 ) {
				Serial.println( ERR "Error (length)" );
				break;
			}
			File	file( SD.open( "db.txt", FILE_WRITE ));
			if( !file ) {
				Serial.println( ERR "Error (open)" );
				break;
			}
			if( !file.seek( code * 24 ) )
				Serial.println( ERR "Error (seek)" );
			else if( file.write( buf ) != RECORD_WIDTH - 1 )
				Serial.println( ERR "Error (file)" );
			else Serial.println( RESP "OK");

			file.close();
		}
		break;

	case 3:		//	SETF <CODE> 0000000
		{
			code = getintparam(inptr);
			if( code == -1 ) {
				Serial.println( ERR "Error (code)" );
				break;
			}
			char* buf = g_serbuf + inptr;
			if( strlen( buf ) != 7 ) {
				Serial.println( ERR "Error (length)" );
				break;
			}
			File	file( SD.open( "db.txt", FILE_WRITE ));
			if(!( 	file &&
					file.seek( code * RECORD_WIDTH + (RECORD_WIDTH - FLAGS_WIDTH - 1) ) &&
					file.write( buf ) == RECORD_WIDTH - FLAGS_WIDTH - 1 ))
				Serial.println( ERR "Error (file)" );
			 else Serial.println( RESP "OK" );
			file.close();
		}
		break;

	case 4:		//	LOG Text to log
		Serial.println( RESP "OK" );
		break;
	}	//	switch
	g_serptr = 0;
}
