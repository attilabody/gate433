#include "database.h"

//////////////////////////////////////////////////////////////////////////////
database::dbrecord::dbrecord()
	: in_start(0)
	, in_end(0)
	, out_start(0)
	, out_end(0)
	, days(0)
	, position( unknown )
{
}

//////////////////////////////////////////////////////////////////////////////
database::dbrecord::dbrecord( const char *&dbstring )
{
	parse( dbstring );
}

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

