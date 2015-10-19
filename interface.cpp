// Do not remove the include below
#include "interface.h"
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library
#include <SPI.h>
#include <SD.h>

#define ITEMCOUNT(A) (sizeof(A)/sizeof(A[0]))
#define BAUDRATE 57600

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

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

// If using the shield, all control and data lines are fixed, and
// a simpler declaration can optionally be used:
// SWTFT tft;

void printCode( int code );
void processInput();

char g_serbuf[64];
unsigned char g_serptr(0);
const char * g_commands[] = {
	  "GET"
	, "SET"
	, "SHOW"
};


void setup(void) {
  Serial.begin( BAUDRATE );
  tft.reset();
  tft.begin( tft.readID() );
  tft.setRotation( 0 );
  delay( 100 );
  tft.fillScreen( BLACK );
  tft.setTextSize(5);
  if( !SD.begin(10) ) {
	  tft.setTextColor(RED);
	  tft.println("SD FAIL");
  }
  tft.setTextColor(GREEN);
  tft.println("SD OK");
}

int getintparam(unsigned char &inptr)
{
  int retval(0);
  bool found(false);
  while (inptr < g_serptr && isdigit(g_serbuf[inptr])) {
    retval *= 10;
    retval += g_serbuf[inptr++] - '0';
    found = true;
  }
  while( inptr < g_serptr && (g_serbuf[inptr] == ' ' || g_serbuf[inptr] == '\n' || g_serbuf[inptr] == ',' )) ++inptr;

  return found ? retval : -1;
}

void loop(void)
{
	while (Serial.available())
	{
		char inc = Serial.read();
		g_serbuf[g_serptr++] = inc;
		if (inc == '\n' || g_serptr == sizeof(g_serbuf)) {
			processInput();
			g_serptr = 0;
		}
	}
}

void printCode( int code ) {
  tft.fillRect(0,0,240,40,BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(WHITE);
  tft.setTextSize(5);
  tft.println(code, DEC);
}

char findcommand(unsigned char &inptr)
{
	while (inptr < g_serptr && g_serbuf[inptr] != ' ' && g_serbuf[inptr] != ','
			&& g_serbuf[inptr] != '\n')
		++inptr;

	if (inptr == g_serptr) return -1;

	for (char i = 0; i < ITEMCOUNT(g_commands); ++i)
	{
		if (!strncmp(g_serbuf, g_commands[i], inptr))
		{
			++inptr;
			while (inptr < g_serptr
					&& (g_serbuf[inptr] == ' ' || g_serbuf[inptr] == '\n')
					|| g_serbuf[inptr] == ',')
				++inptr;
			return i;
		}
	}
	return -1;
}

void processInput()
{
	static char linebuffer[25];

	g_serbuf[ g_serptr ] = 0;

	unsigned char inptr(0);
	int param(0);

	char command = findcommand(inptr);

	switch (command) {
	case 0:		//	GET
		{
			int code( getintparam(inptr));
			if( code == -1 ) break;
			File	file( SD.open( "db.txt", FILE_READ));
			if( !file ) {
				Serial.println( "ERROR" );
				break;
			}
			if( file.seek( code * 24 ) && file.read( linebuffer, 24 ) == 24 ) {
				linebuffer[23] = 0;
				Serial.println( linebuffer );
			}
		}
		break;

	case 1:		//	SET
		break;

	case 2:		//	SHOW
		int code( getintparam(inptr));
		printCode( code );
		break;
	}
}
