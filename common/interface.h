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

struct dbrecord
{
	dbrecord( const char* &dbstring );
	dbrecord();
	bool parse( const char *&dbstring);
	int16_t	in_start;
	int16_t	in_end;
	int16_t	out_start;
	int16_t	out_end;
	uint8_t	days;
	enum POSITION : uint8_t
	{
		  unknown
		, outside
		, inside
	}		position;
};

long getintparam( const char* &input, bool decimal = true, bool trimstart = true );
char findcommand( const char* &inptr, const char **commands );
bool getlinefromserial( char* buffer, uint16_t buflen, uint16_t &idx );
void hex2serial( uint16_t out, uint8_t digits, const char* prefix );
#endif /* _interface_H_ */
