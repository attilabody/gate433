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

dbrecord::dbrecord( const char *&dbstring )
{
	parse( dbstring );
}

bool dbrecord::parse( const char* &dbstring )
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
long getintparam( const char* &input, bool decimal, bool trimstart )
{
	long	retval(0);
	char	converted;
	bool	found(false);

	if( trimstart )
		while( *input && convertdigit( * input, decimal ) == -1 )
			++input;

	while( *input ) {
		if(( converted = convertdigit( *input, decimal )) == -1) break;
		retval *=  decimal ? 10 : 16;
		retval += converted;
		found = true;
		++input;
	}

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
		int cmdlen(0);
		while( inptr[cmdlen] && !isspace(inptr[cmdlen]))
			++cmdlen;
		if (!strncmp(inptr, *commands, cmdlen))
		{
			inptr += cmdlen;
			while( *inptr && (isspace(*inptr) || ispunct(*inptr)) )
				++inptr;
			return ret;
		}
		++ret;
		++commands;
	}
	return -1;
}

//////////////////////////////////////////////////////////////////////////////
bool getlinefromserial( char* buffer, uint16_t buflen, uint16_t &idx )
{
#if defined(DBGSERIALIN)
	if( Serial.available() )
	{
		Serial.print( CMNT "called with buflen ");
		Serial.print( (int)buflen );
		Serial.print( " and idx ");
		Serial.println( (int) idx );
	}
#endif	//	DBGSERIALIN
	bool lineready( false );
	while( Serial.available() && !lineready ) {
		char inc = Serial.read();
#if defined(DBGSERIALIN)
		buffer[idx ] = 0;
		Serial.print( CMNT );
		Serial.print( buffer );
		Serial.print( " " );
		Serial.print( inc );
		Serial.print( ' ' );
		Serial.println( idx );
#endif	//	DBGSERIALIN
		if( inc == '\n' )
			inc = 0;
		buffer[idx++] = inc;
		if( !inc || idx >= buflen - 1 ) {
			if( inc )
				buffer[idx] = 0;
			lineready = true;
#if defined(DBGSERIALIN)
			Serial.print( CMNT "Line ready:" );
			Serial.print( buffer );
			Serial.print( "|" );
			Serial.print( (int)inc );
			Serial.print( " " );
			Serial.println( idx );
			Serial.print( CMNT );
			for( uint8_t _idx = 0; _idx < idx; ++_idx ) {
				Serial.print( (int) buffer[_idx] );
				Serial.print( ' ' );
			}
			Serial.println();
#endif	//	DBGSERIALIN
		}
	}
	return lineready;
}

//////////////////////////////////////////////////////////////////////////////
void hex2serial( uint16_t out, uint8_t digits, const char* prefix )
{
	if( !digits ) return;
	if( digits > 4 ) digits = 4;
	if( prefix ) Serial.print( prefix );
	uint8_t	digit( digits-1 );
	do {
		char	cd( (out >> (digit << 2)) & 0x0f );
		Serial.print((char)( cd < 10 ? cd + '0' : cd - 10 + 'A' ));
	} while( digit-- );
}

//////////////////////////////////////////////////////////////////////////////
inline void halfbytetohex( unsigned char data, char* &buffer ) {
	*buffer++ = data + ( data < 10 ? '0' : ( 'A' - 10 ) );
}

//////////////////////////////////////////////////////////////////////////////
inline void bytetohex( unsigned char data, char* &buffer, bool both ) {
	if( both )
		halfbytetohex( data >> 4, buffer );
	halfbytetohex( data & 0x0f, buffer );
}

//////////////////////////////////////////////////////////////////////////////
void uitohex( uint16_t data, char* &buffer, uint8_t digits ) {
	if( digits > 2 )
		bytetohex( (unsigned char)( data >> 8 ), buffer, digits >= 4 );
	bytetohex( (unsigned char)data, buffer, digits != 1 );
}

//////////////////////////////////////////////////////////////////////////////
void ultohex( uint32_t data, char* &buffer, uint8_t digits ) {
	if( digits > 4 ) {
		uitohex( (uint16_t)( data >> 16 ), buffer, digits - 4 );
		digits -= digits - 4;
	}
	uitohex( (uint16_t)data, buffer, digits );
}

//////////////////////////////////////////////////////////////////////////////
void uitodec( uint16_t data, char* &buffer, uint8_t digits )
{
	char *ptr( buffer + digits - 1 );
	while( digits-- ) {
		*ptr-- = ( data%10 ) + '0';
		data /= 10;
	}
	buffer += digits;
}

//////////////////////////////////////////////////////////////////////////////
void ultodec( uint32_t data, char* &buffer, uint8_t digits )
{
	char *ptr( buffer + digits - 1);
	while( digits-- ) {
		*ptr-- = ( data%10 ) + '0';
		data /= 10;
	}
	buffer += digits;
}

//////////////////////////////////////////////////////////////////////////////
void datetostring( char* &buffer, uint16_t year, uint8_t month, uint8_t day, uint8_t dow, char datesep, char dowsep )
{
	uitodec( year, buffer, 2 ); *buffer++ = datesep;
	uitodec( month, buffer, 2 ); *buffer++ = datesep;
	uitodec( day, buffer, 2 ); *buffer++ = dowsep;
	uitodec( dow, buffer, 1);
}

//////////////////////////////////////////////////////////////////////////////
void timetostring( char* &buffer, uint8_t hour, uint8_t min, uint8_t sec, char sep )
{
	uitodec( hour, buffer, 2 ); *buffer++ = sep;
	uitodec( min, buffer, 2 ); *buffer++ = sep;
	uitodec( sec, buffer, 2 );
}
