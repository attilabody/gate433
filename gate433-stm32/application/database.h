/*
 * database.h
 *
 *  Created on: Nov 11, 2015
 *      Author: abody
 */

#ifndef DATABASE_H_
#define DATABASE_H_
#include <inttypes.h>

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

class database
{
public:
	struct dbrecord
	{
		enum POSITION : uint8_t {
			  UNKNOWN = 0, OUTSIDE , INSIDE
		};
		dbrecord();
		dbrecord( const char* &dbstring );
		dbrecord( uint16_t is, uint16_t ie, uint16_t os, uint16_t oe, uint8_t d, POSITION p );
		bool parse( const char *&dbstring);
		void serializeinfo( char *&buffer ) const;
		void serializestatus( char *&buffer ) const;
		void serialize( char *&buffer ) const;
		void serialize( char *buffer ) const;
		uint8_t pack( uint8_t *buffer ) const;
		void unpack( uint8_t *buffer );
		bool enabled() const { return (in_start < in_end || out_start < out_end ) && (days & 0x7f) != 0; }
		void settimes( uint16_t is, uint16_t ie, uint16_t os, uint16_t oe );
		bool infoequal(const dbrecord &other);

		uint16_t	in_start;
		uint16_t	in_end;
		uint16_t	out_start;
		uint16_t	out_end;
		uint8_t		days;
		POSITION	position;
	};

	virtual bool getParams( int code, dbrecord &recout ) = 0;
	virtual bool setParams( int code, const dbrecord &recin ) = 0;
	virtual bool setInfo( int code, const dbrecord &recin ) = 0;
	virtual bool setStatus( int code, dbrecord::POSITION pos ) = 0;
	virtual void cleanstatuses() = 0;
	virtual ~database() = default;

};

#endif /* DATABASE_H_ */
