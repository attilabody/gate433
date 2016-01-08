/*
 * database.h
 *
 *  Created on: Nov 11, 2015
 *      Author: abody
 */

#ifndef DATABASE_H_
#define DATABASE_H_
#include <Arduino.h>
#include "interface.h"

class database
{
public:
	struct dbrecord
	{
		dbrecord( const char* &dbstring );
		dbrecord();
		bool parse( const char *&dbstring);
		void serializeinfo( char *&buffer ) const;
		void serializestatus( char *&buffer ) const;
		void serialize( char *&buffer ) const;
		void serialize( char *buffer ) const;
		uint8_t pack( uint8_t *buffer ) const;
		void unpack( uint8_t *buffer );

		int16_t	in_start;
		int16_t	in_end;
		int16_t	out_start;
		int16_t	out_end;
		uint8_t	days;
		enum POSITION : uint8_t {
			  unknown , outside , inside
		} position;
	};

	virtual bool getParams( int code, dbrecord &out ) = 0;
	virtual bool setParams( int code, const dbrecord &in ) = 0;
	virtual bool setStatus( int code, dbrecord::POSITION pos ) = 0;
	virtual void cleanstatuses() = 0;
};

#endif /* DATABASE_H_ */
