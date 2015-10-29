/*
 * interface.cpp
 *
 *  Created on: Oct 29, 2015
 *      Author: compi
 */
#include <Arduino.h>
#include "interface.h"

dbrecord::dbrecord()
	: in_start(0)
	, in_end(0)
	, out_start(0)
	, out_end(0)
	, days(0)
	, position( unknown )
{
}

dbrecord::dbrecord( const char *dbstring )
{
}

//////////////////////////////////////////////////////////////////////////////
inline char convertdigit( char c, bool decimal = true )
{
	if( decimal )
		return (c >= '0' && c <= '9') ? c - '0' : -1;
	else {
		if( c >= '0' && c <= '9' ) return c - '0';
		if( c >= 'a' && c <= 'f' ) return c - 'a' + 10;
		if( c >= 'A' && c <= 'F' ) return c - 'A' + 10;
		return -1;
	}
}

//////////////////////////////////////////////////////////////////////////////
long getintparam( const char* &input, bool decimal)
{
	long	retval(0);
	char	converted;
	bool	found(false);

	while( *input && convertdigit( * input, decimal ) == -1 )
		++input;

	while( *input ) {
		if(( converted = convertdigit( *input++, decimal )) == -1) break;
		retval *=  decimal ? 10 : 16;
		retval += converted;
		found = true;
	}
	while( *input )
		if( isspace( *input ) || ispunct( *input ))
			++input;

	return found ? retval : -1;
}

//////////////////////////////////////////////////////////////////////////////
char findcommand( const char* &inptr, const char **commands )
{
	int		ret(0);
	while( *inptr && (isspace(*inptr) || ispunct(*inptr)) )
		++inptr;
	if( !*inptr ) return -1;

	while( *commands )
	{
		if (!strcmp(inptr, *commands ))
		{
			while( *inptr && (isspace(*inptr) || ispunct(*inptr)) )
				++inptr;
			return ret;
		}
		++ret;
		++commands;
	}
	return -1;
}

