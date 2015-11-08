// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section
#ifndef _interface_H_
#define _interface_H_
//#define DBGSERIALIN	1

#define ITEMCOUNT(A) (sizeof(A)/sizeof(A[0]))

#define BAUDRATE 57600
#define RESP ':'
#define ERR '!'
#define CMNT '#'
#define RESPS ":"
#define ERRS "!"
#define CMNTS "#"

//template< typename T1, typename T2 > void serialout( const T1 &a, const T2 &b, bool newline = true )
//{
//	Serial.print( a );
//	Serial.print( b );
//	if( newline ) Serial.println();
//	else Serial.print( " - " );
//}
//
//template< typename T1, typename T2, typename T3> void serialout( const T1 &a, const T2 &b, const T3 &c, bool newline = true )
//{
//	Serial.print( a );
//	Serial.print( b );
//	Serial.print( c );
//	if( newline ) Serial.println();
//	else Serial.print( " - " );
//}
//
//template< typename T1, typename T2, typename T3, typename T4> void serialout( const T1 &a, const T2 &b, const T3 &c, const T4 &d, bool newline = true )
//{
//	Serial.print( a );
//	Serial.print( b );
//	Serial.print( c );
//	Serial.print( d );
//	if( newline ) Serial.println();
//	else Serial.print( " - " );
//}

inline void serialout() { Serial.println(); }
template< typename Arg1, typename... Args> void serialout( const Arg1& arg1, const Args&... args)
{
	Serial.print( arg1 );
	serialout( args...);
}


/*
000 59F 000 59F 000007F
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

struct dbrecord
{
	dbrecord( const char* dbstring );
	dbrecord();
	uint16_t	in_start;
	uint16_t	in_end;
	uint16_t	out_start;
	uint16_t	out_end;
	uint8_t		days;
	enum POSITION : uint8_t
	{
		  unknown
		, outside
		, inside
	}			position;
};

long getintparam( const char* &input, bool decimal = true, bool ff = true );
char findcommand( const char* &inptr, const char **commands );
bool getlinefromserial( char* buffer, uint16_t buflen, uint16_t &idx );

#endif /* _interface_H_ */
