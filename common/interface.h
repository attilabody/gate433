// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section
#ifndef _interface_H_
#define _interface_H_
//#define DBGSERIALIN	1
#include <Arduino.h>

struct ts;

#define  DEBUGTEMPLATES

#define ITEMCOUNT(A) (sizeof(A)/sizeof(A[0]))

#define RESP ':'
#define RESPS ":"
#define ERR '!'
#define ERRS "!"
#define CMNT '#'
#define CMNTS "#"

#ifdef DEBUGTEMPLATES

template<typename Sep, typename Arg1> void serialoutsepln( const Sep sep, const Arg1 arg1 ) {
	Serial.print( arg1 );
	Serial.println();
}
template< typename Sep, typename Arg1, typename... Args> void serialoutsepln(const Sep sep, const Arg1& arg1, const Args&... args) {
	Serial.print( arg1 );
	Serial.print( sep );
	serialoutsepln( sep, args...);
}

template<typename Sep, typename Arg1> void serialoutsep( const Sep sep, const Arg1 arg1 ) {
	Serial.print( arg1 );
	Serial.print( sep );
}
template< typename Sep, typename Arg1, typename... Args> void serialoutsep(const Sep sep, const Arg1& arg1, const Args&... args) {
	Serial.print( arg1 );
	Serial.print( sep );
	serialoutsep( sep, args...);
}

template< typename Arg1 > void serialoutln( const Arg1& arg1 )
{
	Serial.println( arg1 );
}

template< typename Arg1, typename... Args> void serialoutln( const Arg1& arg1, const Args&... args)
{
	Serial.print( arg1 );
	serialoutln( args...);
}

template< typename Arg1 > void serialout( const Arg1& arg1 )
{
	Serial.print( arg1 );
}

template< typename Arg1, typename... Args> void serialout( const Arg1& arg1, const Args&... args)
{
	Serial.print( arg1 );
	serialout( args...);
}
#endif	//	DEBUGTEMPLATES

/*
INS INE OUS OUE FLG
000 59F 000 59F 07F
*/
#define IN_START_OFFSET			0
#define IN_START_WIDTH			3
#define IN_END_OFFSET			(IN_START_OFFSET + IN_START_WIDTH + 1)
#define IN_END_WIDTH			3
#define OUT_START_OFFSET		(IN_END_OFFSET + IN_END_WIDTH + 1)
#define OUT_START_WIDTH			3
#define OUT_END_OFFSET			(OUT_START_OFFSET + OUT_START_WIDTH + 1)
#define OUT_END_WIDTH			3
#define FLAGS_OFFSET			(OUT_END_OFFSET + OUT_END_WIDTH + 1)
#define FLAGS_WIDTH				3
#define INFORECORD_WIDTH		(FLAGS_OFFSET + FLAGS_WIDTH + 1)	// \n added
#define STATUS_OFFSET			(FLAGS_OFFSET + FLAGS_WIDTH + 1)
#define STATUS_WIDTH			3
#define DBRECORD_WIDTH			(STATUS_OFFSET + STATUS_WIDTH + 1)	// \n added
#define PACKEDDBRECORD_WIDTH	8

#define STATUSRECORD_WIDTH		4

long getintparam( const char* &input, bool decimal = true, bool trimstart = true, bool acceptneg = false );
bool iscommand( const char *&inptr, const char *cmd, bool pgmspace = true );
bool iscommand( const char *&inptr, const __FlashStringHelper *cmd );
char findcommand( const char* &inptr, const char **commands );
bool getlinefromserial( char* buffer, uint8_t buflen, uint8_t &idx );
void hex2serial( uint16_t out, uint8_t digits, const char* prefix );
uint8_t uitohex( char* buffer, uint16_t data, uint8_t digits );
uint8_t ultohex( char* buffer, uint32_t data, uint8_t digits );
uint8_t uitodec( char* buffer, uint16_t data, uint8_t digits );
uint8_t ultodec( char* buffer, uint32_t data, uint8_t digits );
void datetostring( char* &buffer, uint16_t year, uint8_t month, uint8_t day, uint8_t dow, uint8_t yeardigits = 2, bool showdow = false, char datesep = '.', char dowsep = '/' );
void timetostring( char* &buffer, uint8_t hour, uint8_t min, uint8_t sec, char sep );
void printdate( Print *p, uint16_t year, uint8_t month, uint8_t day, uint8_t dow, uint8_t yeardigits = 2, bool showdow = false, char datesep = '.', char dowsep = '/' );
void printtime( Print *p, uint8_t hour, uint8_t min, uint8_t sec, char sep );
bool parsedatetime( ts &t, const char *&inptr );
inline char halfbytetohex( uint8_t data ) { return data + ( data < 10 ? '0' : ( 'A' - 10 ) ); }



#endif /* _interface_H_ */
