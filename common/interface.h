// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section
#ifndef _interface_H_
#define _interface_H_
//#define DBGSERIALIN	1
#include <ds3231.h>

#define ITEMCOUNT(A) (sizeof(A)/sizeof(A[0]))

#define BAUDRATE 57600
#define RESP ':'
#define RESPS ":"
#define ERR '!'
#define ERRS "!"
#define CMNT '#'
#define CMNTS "#"

#define CMD_GET		"get"
#define CMD_SET		"set"
#define CMD_SETS	"sets"
#define CMD_LOG		"log"

inline void serialoutln() { Serial.println(); }
template< typename Arg1, typename... Args> void serialoutln( const Arg1& arg1, const Args&... args)
{
	Serial.print( arg1 );
	serialoutln( args...);
}

inline void serialout() {}
template< typename Arg1, typename... Args> void serialout( const Arg1& arg1, const Args&... args)
{
	Serial.print( arg1 );
	serialout( args...);
}

/*
INS INE OUS OUE FLG
000 59F 000 59F 07F
*/
#define IN_START_OFFSET		0
#define IN_START_WIDTH		3
#define IN_END_OFFSET		(IN_START_OFFSET + IN_START_WIDTH + 1)
#define IN_END_WIDTH		3
#define OUT_START_OFFSET	(IN_END_OFFSET + IN_END_WIDTH + 1)
#define OUT_START_WIDTH		3
#define OUT_END_OFFSET		(OUT_START_OFFSET + OUT_START_WIDTH + 1)
#define OUT_END_WIDTH		3
#define FLAGS_OFFSET		(OUT_END_OFFSET + OUT_END_WIDTH + 1)
#define FLAGS_WIDTH			3
#define INFORECORD_WIDTH	(FLAGS_OFFSET + FLAGS_WIDTH + 1)	// \n added

#define STATUSRECORD_WIDTH	4

long getintparam( const char* &input, bool decimal = true, bool trimstart = true );
char findcommand( const char* &inptr, const char **commands );
bool getlinefromserial( char* buffer, uint16_t buflen, uint16_t &idx );
void hex2serial( uint16_t out, uint8_t digits, const char* prefix );
void uitohex( char* &buffer, uint16_t data, uint8_t digits );
void ultohex( char* &buffer, uint32_t data, uint8_t digits );
void uitodec( char* &buffer, uint16_t data, uint8_t digits );
void ultodec( char* &buffer, uint32_t data, uint8_t digits );
void datetostring( char* &buffer, uint16_t year, uint8_t month, uint8_t day, uint8_t dow, char datesep, char dowsep );
void timetostring( char* &buffer, uint8_t hour, uint8_t min, uint8_t sec, char sep );
bool parsedatetime( ts &t, const char *&inptr );

#endif /* _interface_H_ */
