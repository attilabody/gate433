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

Adafruit_TFTLCD g_tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

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

void	printCode( int code, uint16_t fgcolor = WHITE, uint16_t bgcolor = BLACK );
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

char 			g_inbuf[256];
unsigned char	g_inidx(0);
//char 			g_outbuf[256];
//unsigned char	g_outidx(0);
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
	g_tft.reset();
	g_tft.begin(g_tft.readID());
	g_tft.setRotation(2);
	delay(100);
	g_tft.fillScreen( BLACK);

	g_tft.setTextSize(5);
	if (!SD.begin(10)) {
		g_tft.setTextColor(RED);
		g_tft.println("SD FAIL");
	} else {
		g_tft.setTextColor(GREEN);
		g_tft.println("SD OK");
	}
	g_buttonheight = 40;
	g_buttontop = g_tft.height() - g_buttonheight;
	g_buttonwidth = g_tft.width() / 2;

	drawbuttons( true );
	drawbuttons( true );
}

//////////////////////////////////////////////////////////////////////////////
void loop(void)
{
	if( getlinefromserial()) {
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
		g_inbuf[g_inidx ] = 0;
		Serial.print( CMNT );
		Serial.print( " " );
		Serial.println( g_inbuf );
		Serial.print( inc );
		Serial.print( ' ' );
		Serial.println( g_inidx );
#endif	//	DBGSERIALIN
		if( inc == '\n') inc = 0;
		g_inbuf[g_inidx++] = inc;
		if ( !inc || g_inidx >= sizeof( g_inbuf ) -1 )
		{
			if( inc ) g_inbuf[g_inidx] = 0;
			lineready = true;
#if defined(DBGSERIALIN)
			Serial.print( CMNT "Line ready:" );
			Serial.print( g_inbuf );
			Serial.print( "|" );
			Serial.print( (int)inc );
			Serial.print( " " );
			Serial.println( g_inidx );
			Serial.print( CMNT );
			for( char idx = 0; idx < g_inidx; ++idx ) {
				Serial.print( (int) g_inbuf[idx] );
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
	g_tft.fillRect(0, g_buttontop, g_buttonwidth, g_buttonheight, first ? color_h : color_l );
	g_tft.fillRect(g_buttonwidth, g_buttontop, g_buttonwidth, g_buttonheight, first ? color_l : color_h );
	g_tft.setTextSize(3);
	g_tft.setCursor( 27, g_buttontop + 8 ); g_tft.setTextColor( first ? WHITE : GREY ); g_tft.print( "lako" );
	g_tft.setCursor( g_buttonwidth + 17, g_buttontop + 8); g_tft.setTextColor( first ? GREY : WHITE ); g_tft.print("berlo");
}

//////////////////////////////////////////////////////////////////////////////
void printCode( int code, uint16_t color, uint16_t bgcolor )
{
	g_tft.fillRect( 0, 0, 240, 40, bgcolor );
	g_tft.setCursor( 0, 0 );
	g_tft.setTextColor( color );
	g_tft.setTextSize( 5 );
	g_tft.println( code, DEC );
}

//////////////////////////////////////////////////////////////////////////////
void printInput()
{
	g_tft.fillRect( 0, 40, 240, 240, BLACK );
	g_tft.setCursor( 0, 42 );
	g_tft.setTextColor( WHITE );
	g_tft.setTextSize( 1 );
	g_tft.println( g_inbuf );
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


	while( sbidx < g_inidx ) {
		if(( converted = convertdigit( g_inbuf[sbidx++])) == -1) break;
		retval *=  decimal ? 10 : 16;
		retval += converted;
		found = true;
	}
	while (sbidx < g_inidx
			&& (g_inbuf[sbidx] == ' ' || g_inbuf[sbidx] == '\n'
					|| g_inbuf[sbidx] == ',')) {
		++sbidx;
	}

	return found ? retval : -1;
}

//////////////////////////////////////////////////////////////////////////////
char findcommand(unsigned char &inptr)
{
	while (inptr < g_inidx && g_inbuf[inptr] && g_inbuf[inptr] != ' ' && g_inbuf[inptr] != ','
			&& g_inbuf[inptr] != '\n')
		++inptr;
	if (inptr == g_inidx) return -1;

	for (char i = 0; i < ITEMCOUNT(g_commands); ++i)
	{
		if (!strncmp(g_inbuf, g_commands[i], inptr))
		{
			while(	inptr < g_inidx &&
					( g_inbuf[inptr] == ' ' || g_inbuf[inptr] == '\n' || g_inbuf[inptr] == ',' )
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
	printInput();

	static char 	linebuffer[26];
	unsigned char	inptr(0);
	char			command( findcommand(inptr) );
	int				code( 0 );
	const char		*output( NULL );

	switch (command) {
	default:
		output = ERR "Error (command)";
		break;
	case 0:		//	CODE <CODE>
		{
			code = getintparam(inptr);

			if( code == -1 ) {
				output = ERR "Error (code)";
				break;
			}
			File	file;
			if( g_recordmode )
			{
				file = SD.open( "db.txt", FILE_WRITE );
				if( !file ) {
					output = ERR "Error (open)";
					break;
				}
				output = g_owner ? ":000 59F 000 59F 000007F" : ":1E0 455 1E0 455 000001F";
				if( !file.seek( code * RECORD_WIDTH ) )
					output = ERR "Error (seek)";
				else if( file.write( output + 1 ) != RECORD_WIDTH -1  )
					output = ERR "Error (file)";
			}
			else
			{
				file = SD.open( "db.txt", FILE_READ );
				if( !file ) {
					output = ERR "Error (open)";
					break;
				}
				if( !file.seek( code * RECORD_WIDTH ))
					output = ERR "Error (seek)";
				else if( file.read( linebuffer+1, RECORD_WIDTH ) != RECORD_WIDTH )
					output = ERR "Error (read)";
				else {
					output = linebuffer;
					linebuffer[0] = RESP[0];
					linebuffer[24] = 0;	//	clamp trailing \n
				}
			}
			file.close();
		}
		break;

	case 1:		//	GET <CODE>
		{
			code = getintparam(inptr);
			if( code == -1 ) {
				output = RESP "Error (code)";
				break;
			}
			File	file( SD.open( "db.txt", FILE_READ ));
			if( !file ) {
				output = RESP "Error (open)";
				break;
			}
			if( file.seek( code * RECORD_WIDTH )) {
				if( file.read( linebuffer+1, RECORD_WIDTH ) == RECORD_WIDTH ) {
					output = linebuffer;
					linebuffer[0] = RESP[0];
					linebuffer[24] = 0;
				} else
					output = RESP "Error (read)";
			} else
				output = RESP "Error (seek)";
			file.close();
		}
		break;

	case 2:		//	SET <CODE> 000 59F 000 59F 0000000
		{
			code = getintparam(inptr);
			if( code == -1 ) {
				output = ERR "Error (code)";
				break;
			}
			char* buf = g_inbuf + inptr;
			if( strlen( buf ) != RECORD_WIDTH - 1 ) {
				output = ERR "Error (length)";
				break;
			}
			File	file( SD.open( "db.txt", FILE_WRITE ));
			if( !file ) {
				output = ERR "Error (open)";
				break;
			}
			if( !file.seek( code * 24 ) )
				output = ERR "Error (seek)";
			else if( file.write( buf ) != RECORD_WIDTH - 1 )
				output = ERR "Error (file)";
			else output = RESP "OK";

			file.close();
		}
		break;

	case 3:		//	SETF <CODE> 0000000
		{
			code = getintparam(inptr);
			if( code == -1 ) {
				output = ERR "Error (code)";
				break;
			}
			char* buf = g_inbuf + inptr;
			if( strlen( buf ) != 7 ) {
				output = ERR "Error (length)";
				break;
			}
			File	file( SD.open( "db.txt", FILE_WRITE ));
			if(!( 	file &&
					file.seek( code * RECORD_WIDTH + (RECORD_WIDTH - FLAGS_WIDTH - 1) ) &&
					file.write( buf ) == RECORD_WIDTH - FLAGS_WIDTH - 1 ))
				output = ERR "Error (file)";
			 else output = RESP "OK";
			file.close();
		}
		break;

	case 4:		//	LOG Text to log
		output = RESP "OK";
		break;
	}	//	switch

	if( output ) {
		g_tft.println( output );
		Serial.println( output );
		if( code != -1 )
			printCode( code, output[0] == RESP[0] ? WHITE : RED );
	}
	g_inidx = 0;
}
