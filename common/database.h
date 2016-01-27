/*
 * database.h
 *
 *  Created on: Nov 11, 2015
 *      Author: abody
 */

#ifndef DATABASE_H_
#define DATABASE_H_
#include <Arduino.h>

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

		uint16_t	in_start;
		uint16_t	in_end;
		uint16_t	out_start;
		uint16_t	out_end;
		uint8_t		days;
		POSITION	position;
	};

	virtual bool getParams( int code, dbrecord &out ) = 0;
	virtual bool setParams( int code, const dbrecord &in ) = 0;
	virtual bool setStatus( int code, dbrecord::POSITION pos ) = 0;
	virtual void cleanstatuses() = 0;
};

#endif /* DATABASE_H_ */
