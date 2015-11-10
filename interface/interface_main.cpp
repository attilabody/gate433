// Do not remove the include below
#include "Arduino.h"
#include <interface.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library
#include <TouchScreen.h>
#include <SdFat.h>

#define VERBOSE
//#define VERBOSE_SETUP
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
#define YM 9   // can be a digital pin
#define XM A2  // must be an analog pin, use "An" notation!
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
void	drawbuttons( bool first, uint16_t color_h = D_GREEN, uint16_t color_l = D_GREY );
void	printInput();

char 			g_inbuf[256];
uint16_t		g_inidx(0);

const char 		*g_commands[] = {
	  "GET"
	, "SET"
	, "SETS"
	, "LOG"
	, ""
};

uint16_t	g_buttonheight, g_buttonwidth, g_buttontop;
SdFat		g_sd;
File		g_info;
bool		g_initok(false);

//////////////////////////////////////////////////////////////////////////////
void setup(void)
{
	static char srcname[] = "backup_9.txt";
	static char dstname[] = "backup_9.txt";

	Serial.begin( BAUDRATE);
	g_tft.reset();
	g_tft.begin(g_tft.readID());
	g_tft.setRotation(2);
	delay(100);
	g_tft.fillScreen( BLACK);

	g_tft.setTextSize(5);
	bool	sdsucc(true);
	if( g_sd.begin( SS, SPI_HALF_SPEED )) {
		g_info = g_sd.open( "info.txt", FILE_READ );
		sdsucc = g_info;
	}

	if( sdsucc ) {
		g_tft.setTextColor(GREEN);
		g_tft.println("SD OK");
		g_initok = true;
	} else {
		g_tft.setTextColor(RED);
		g_tft.println("SD FAIL");
		return;
	}
	srcname[7] = '9';
#ifdef VERBOSE_SETUP
	serialout( "Removing ", srcname );
	delay(100);
#endif
	g_sd.remove( srcname );
	for( char id = '8'; id >= '0'; --id ) {
		srcname[7] = id;
		dstname[7] = id + 1;
#ifdef VERBOSE_SETUP
		serialout( "Renaming ", srcname, " to ", dstname);
		delay(100);
#endif
		g_sd.rename( srcname, dstname );
	}
	File	df( g_sd.open( srcname, FILE_WRITE ));
	File	sf( g_sd.open( "status.txt", FILE_READ ));
	int		nread;

	uint32_t	total(0);
	if( sf && df ) {
		while( (nread = sf.read(g_inbuf, sizeof( g_inbuf ))) > 0 ) {
			df.write( g_inbuf, nread );
			total += nread;
		}
#ifdef VERBOSE_SETUP
		serialout( "Copied ", total, " bytes" );
#endif
	}
	if( sf ) sf.close();
#ifdef VERBOSE_SETUP
	else Serial.println("Opening status.txt failed");
#endif
	if( df ) df.close();
#ifdef VERBOSE_SETUP
	else Serial.println("Opening backup_0.txt failed");
#endif

	g_buttonheight = 40;
	g_buttontop = g_tft.height() - g_buttonheight;
	g_buttonwidth = g_tft.width() / 2;

	drawbuttons( true );
	drawbuttons( true );
}

//////////////////////////////////////////////////////////////////////////////
void loop(void)
{
	if( !g_initok ) return;

	if( getlinefromserial( g_inbuf, sizeof(g_inbuf), g_inidx )) {
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
void processInput()
{
	printInput();
	if( g_inbuf[0] == CMNT ) {
		g_inidx = 0;
		return;
	}
#ifdef VERBOSE
	else serialout( CMNT, g_inbuf );
#endif	//	VERBOSE

	static char 	linebuffer[INFORECORD_WIDTH + STATUSRECORD_WIDTH + 2];
	const char		*inptr(g_inbuf);
	char			command( findcommand( inptr, g_commands ));
	int				code(-1);
	const char		*output( ERRS "Error (init)" );
	File			status;

	if( g_initok )
	{
		switch (command)
		{
		default:
			output = ERRS "Error (command)";
			break;
		case 0:		//	GET <CODE>
			{
				code = getintparam(inptr);
				if( code == -1 ) {
					output = ERRS "Error (code)";
					break;
				}
				status = g_sd.open( "status.txt", FILE_READ );
				if( !status ) {
					output = ERRS "Error (open status)";
					break;
				}
#if false
//#ifdef VERBOSE
				serialout( CMNTS " ", code, " ", code * INFORECORD_WIDTH, " ", code * STATUSRECORD_WIDTH );
				memset( linebuffer, 0, sizeof(linebuffer) );
				linebuffer[0] = ' ';
#endif
				if( !g_info.seek( code * INFORECORD_WIDTH )) {
					output = ERRS "Error (seek info)";
				} else if( !status.seek( code * STATUSRECORD_WIDTH )) {
					output = ERRS "Error (seek status)";
				} else if( g_info.read( linebuffer+1, INFORECORD_WIDTH ) != INFORECORD_WIDTH ) {
					output = ERRS "Error (read info)";
				} else if( status.read( linebuffer + 1 + INFORECORD_WIDTH, STATUSRECORD_WIDTH ) != STATUSRECORD_WIDTH) {
					output = ERRS "Error (read status)";
				} else {
					output = linebuffer;
					linebuffer[0] = RESP;
					linebuffer[INFORECORD_WIDTH] = ' ';						//	replace \n with ' '
					linebuffer[INFORECORD_WIDTH + STATUSRECORD_WIDTH] = 0;	//	clamp trailing \n
				}
				status.close();
			}
			break;

		case 1:		//	SET <CODE> 000 59F 000 59F 0000000
			{
				code = getintparam(inptr);
				if( code == -1 ) {
					output = ERRS "Error (code)";
					break;
				}

				while( *inptr && isspace( *inptr )) ++inptr;

				if( strlen( inptr ) != INFORECORD_WIDTH + STATUSRECORD_WIDTH - 1 ) {
					output = ERRS "Error (length)";
					break;
				}
				g_info.close();
				if( !(g_info = g_sd.open( "info.txt", FILE_WRITE ))) {
					output = ERRS "Error (open info)";
					g_initok = false;
				} else if( !(status = g_sd.open( "status.txt", FILE_WRITE ))) {
					output = ERRS "Error (open status)";
				} else if( !g_info.seek( code * INFORECORD_WIDTH ) ) {
					output = ERRS "Error (seek info)";
				} else if( !status.seek(code * STATUSRECORD_WIDTH)) {
					output = ERRS "Error (seek status)";
				} else if( g_info.write( inptr, INFORECORD_WIDTH - 1 ) != INFORECORD_WIDTH - 1 ) {
					output = ERRS "Error (write info)";
				} else if(status.write( inptr + INFORECORD_WIDTH, STATUSRECORD_WIDTH -1 ) != STATUSRECORD_WIDTH -1 ) {
					output = ERRS "Error (write status)";
				} else output = RESPS "OK";

				status.close();
				g_info.close();
				if( !(g_info = g_sd.open( "info.txt", FILE_READ )))
					g_initok = false;
			}
			break;

		case 2:		//	SETS <CODE> 0000000
			{
				code = getintparam(inptr);
				if( code == -1 ) {
					output = ERRS "Error (code)";
					break;
				}

				while( *inptr && isspace( *inptr )) ++inptr;

				if( strlen( inptr ) != STATUSRECORD_WIDTH - 1 ) {
					output = ERRS "Error (length)";
				} else if( !(status = g_sd.open( "status.txt", FILE_WRITE ))) {
					output = ERRS "Error (open status)";
				} else if( !( status.seek( code * STATUSRECORD_WIDTH))) {
					output = ERRS "Error (seek status)";
				} else if( status.write( inptr, STATUSRECORD_WIDTH - 1 ) != STATUSRECORD_WIDTH - 1 ) {
					output = ERRS "Error (write status)";
				} else output = RESPS "OK";
				status.close();
			}
			break;

		case 3:		//	LOG Text to log
			output = RESPS "OK";
			break;
		}	//	switch
	}	//	if( g_initok )

	g_tft.println( output );
	Serial.println( output );
	if( code != -1 )
		printCode( code, output[0] == RESP ? WHITE : RED );

	g_inidx = 0;
}
