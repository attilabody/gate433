#include "database.h"
#define VERBOSE

//////////////////////////////////////////////////////////////////////////////
database::dbrecord::dbrecord()
	: in_start(0)
	, in_end(0)
	, out_start(0)
	, out_end(0)
	, days(0)
	, position( UNKNOWN )
{
}

//////////////////////////////////////////////////////////////////////////////
database::dbrecord::dbrecord( const char *&dbstring )
{
	parse( dbstring );
}

//////////////////////////////////////////////////////////////////////////////
database::dbrecord::dbrecord( uint16_t is, uint16_t ie, uint16_t os, uint16_t oe, uint8_t d, POSITION p )
: in_start(is)
, in_end(ie)
, out_start(os)
, out_end(oe)
, days(d)
, position(p)
{}

//////////////////////////////////////////////////////////////////////////////
bool database::dbrecord::parse( const char* &dbstring )
{
	int16_t	sflags, dflags;

	if( (in_start = getintparam( dbstring, false, true )) == - 1
			|| (in_end = getintparam( dbstring, false, true )) ==  -1
			|| (out_start = getintparam( dbstring, false, true )) ==  -1
			|| (out_end = getintparam( dbstring, false, true )) ==  -1
			|| (sflags = getintparam( dbstring, false, true )) ==  -1
			|| (dflags = getintparam( dbstring, false, true )) ==  -1
	) {
		in_start = -1;
		return false;
	} else {
		days = sflags & 0x7f;
		position = (POSITION)(dflags & 3);
		return true;
	}

}

//////////////////////////////////////////////////////////////////////////////
void database::dbrecord::serializeinfo( char *&buffer ) const
{
	uitohex( buffer, in_start, 3 ); *buffer++ = ' ';
	uitohex( buffer, in_end, 3 ); *buffer++ = ' ';
	uitohex( buffer, out_start, 3 ); *buffer++ = ' ';
	uitohex( buffer, out_end, 3 ); *buffer++ = ' ';
	uitohex( buffer, (uint16_t)days, 3);
}

//////////////////////////////////////////////////////////////////////////////
void database::dbrecord::serializestatus( char *&buffer ) const
{
	uitohex( buffer, (uint16_t) position, 3 );
}

//////////////////////////////////////////////////////////////////////////////
void database::dbrecord::serialize( char *&buffer ) const
{
	serializeinfo( buffer );
	*buffer++ = ' ';
	serializestatus( buffer );
}

//////////////////////////////////////////////////////////////////////////////
void database::dbrecord::serialize( char *buffer ) const
{
	serializeinfo( buffer );
	*buffer++ = ' ';
	serializestatus( buffer );
	*buffer = 0;
}

//////////////////////////////////////////////////////////////////////////////
uint8_t database::dbrecord::pack( uint8_t *buffer ) const
{
	uint32_t	tmp((((uint32_t)in_start) << 12) | (in_end & 0xffff));
	for( uint8_t pos = 0; pos < 3; ++pos ) {
		*buffer++ = (uint8_t)(tmp & 0xff);
		tmp >>= 8;
	}
	tmp = (((uint32_t)out_start) << 12) | (out_end & 0xffff);
	for( uint8_t pos = 0; pos < 3; ++pos ) {
		*buffer++ = (uint8_t)(tmp & 0xff);
		tmp >>= 8;
	}
	*buffer++ = days;
	*buffer++ = position;
#ifdef VERBOSE
	buffer -= 8;
	for( uint8_t pos = 0; pos < 8; ++pos ) {
		if( pos) Serial.print(' ');
		Serial.print( halfbytetohex( buffer[pos] >> 4));
		Serial.print( halfbytetohex( buffer[pos] & 0xf));
	}
	Serial.println();
#endif	//	VERBOSE

	return 8;
}

//////////////////////////////////////////////////////////////////////////////
void database::dbrecord::unpack( uint8_t *buffer )
{
	uint32_t	tmp(0);
	for( uint8_t pos = 0; pos < 3; ++pos ) {
		tmp <<= 8;
		tmp |= buffer[2-pos];
	}
	buffer += 3;
	in_end = tmp & 0xfff;
	in_start = tmp >> 12;

	tmp = 0;
	for( uint8_t pos = 0; pos < 3; ++pos ) {
		tmp <<= 8;
		tmp |= buffer[2-pos];
	}
	buffer += 3;
	out_end = tmp & 0xfff;
	out_start = tmp >> 12;

	days = *buffer++;
	position = (POSITION)*buffer++;
}
