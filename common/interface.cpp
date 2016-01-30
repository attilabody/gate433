/*
 * interface.cpp
 *
 *  Created on: Oct 29, 2015
 *      Author: compi
 */
#include <Arduino.h>
#include "interface.h"
#include <limits.h>
#include <ds3231.h>

//#define DBGSERIALIN

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
long getintparam( const char* &input, bool decimal, bool trimstart, bool acceptneg )
{
	long	retval(0);
	char	converted;
	bool	found(false);
	bool	negative(false);

	if( trimstart )
		while( *input && convertdigit( *input, decimal ) == -1 && *input != '-' )
			++input;

	while( *input ) {
		if(( converted = convertdigit( *input, decimal )) == -1) {
			if( ! retval ) {
				if( *input == 'x' || *input == 'X' ) {
					decimal = false;
					converted = 0;
				} else if( *input == '-' ) {
					negative = true;
					converted = 0;
				} else break;
			} else break;
		}
		retval *=  decimal ? 10 : 16;
		retval += converted;
		found = true;
		++input;
	}
	return found ? (negative ? 0 - retval : retval ) : (acceptneg ? LONG_MIN : -1);
}


//////////////////////////////////////////////////////////////////////////////
bool iscommand( const char *&inptr, const char *cmd, bool pgmspace )
{
	size_t n = 0;
	char x;
	if( pgmspace )
		while((x = pgm_read_byte( cmd++ )) && x == inptr[n] )
			++n;
	else
		while((x = *cmd++) && x == inptr[n] )
			++n;
	char y(inptr[n]);
	if( x || !(isspace(y) || ispunct(y) || y == '\n' || !y ))
		return false;
	inptr += n;
	while( *inptr && (isspace(*inptr)))
		++inptr;
	return true;
}

//////////////////////////////////////////////////////////////////////////////
bool iscommand( const char *&inptr, const __FlashStringHelper *cmd )
{
	PGM_P p = reinterpret_cast<PGM_P>(cmd);
	size_t n = 0;
	char x;
	while((x = pgm_read_byte( p++ )) && x == inptr[n] )
		++n;
	char y(inptr[n]);
	if( x || !(isspace(y) || ispunct(y) || y == '\n' || !y ))
		return false;
	inptr += n;
	while( *inptr && (isspace(*inptr)))
		++inptr;
	return true;
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
			while( *inptr && (isspace(*inptr)))
				++inptr;
			return ret;
		}
		++ret;
		++commands;
	}
	return -1;
}

//////////////////////////////////////////////////////////////////////////////
bool getlinefromserial( char* buffer, uint8_t buflen, uint8_t &idx )
{
#if defined(DBGSERIALIN)
	if( Serial.available() )
	{
		Serial.print( CMNTS "called with buflen ");
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
		if( inc == '\r' ) continue;
		if( inc == '\n' ) inc = 0;
		buffer[idx++] = inc;
		if( !inc || idx >= buflen - 1 ) {
			if( inc )
				buffer[idx] = 0;
			lineready = true;
#if defined(DBGSERIALIN)
			Serial.print( CMNTS "Line ready:" );
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
uint16_t bytetohex( char* buffer, unsigned char data, bool both )
{
	if( both )
		*buffer++ = halfbytetohex( data >> 4 );
	*buffer = halfbytetohex( data & 0x0f );
	return both ? 2 :1;
}

//////////////////////////////////////////////////////////////////////////////
uint8_t uitohex( char* buffer, uint16_t data, uint8_t digits )
{
	if( digits > 2 )
		buffer += bytetohex( buffer, (unsigned char)( data >> 8 ), digits >= 4 );
	bytetohex( buffer, (unsigned char)data, digits != 1 );
	return digits;
}

//////////////////////////////////////////////////////////////////////////////
uint8_t ultohex( char* buffer, uint32_t data, uint8_t digits )
{
	if( digits > 4 ) {
		buffer += uitohex( buffer, (uint16_t)( data >> 16 ), digits - 4 );
		digits -= digits - 4;
	}
	uitohex( buffer, (uint16_t)data, digits );
	return digits;
}

//////////////////////////////////////////////////////////////////////////////
uint8_t uitodec( char* buffer, uint16_t data, uint8_t digits )
{
	buffer += digits - 1;
	uint8_t	cntr( digits );
	while( cntr-- ) {
		*buffer-- = ( data%10 ) + '0';
		data /= 10;
	}
	return digits;
}

//////////////////////////////////////////////////////////////////////////////
uint8_t ultodec( char* buffer, uint32_t data, uint8_t digits )
{
	buffer += digits - 1;
	uint8_t	cntr( digits );
	while( cntr-- ) {
		*buffer-- = ( data%10 ) + '0';
		data /= 10;
	}
	buffer += digits;
	return digits;
}

//////////////////////////////////////////////////////////////////////////////
void datetostring( char* &buffer, uint16_t year, uint8_t month, uint8_t day, uint8_t dow, uint8_t yeardigits, bool showdow, char datesep, char dowsep )
{
	if( yeardigits ) {
		if( yeardigits > 4 ) yeardigits = 4;
		buffer += uitodec( buffer, year, yeardigits ); *buffer++ = datesep;
	}
	buffer += uitodec( buffer, month, 2 ); *buffer++ = datesep;
	buffer += uitodec( buffer, day, 2 );
	if( showdow ) {
		*buffer++ = dowsep;
		buffer += uitodec( buffer, dow, 1);
	}
}

//////////////////////////////////////////////////////////////////////////////
void timetostring( char* &buffer, uint8_t hour, uint8_t min, uint8_t sec, char sep )
{
	buffer += uitodec( buffer, hour, 2 ); *buffer++ = sep;
	buffer += uitodec( buffer, min, 2 ); *buffer++ = sep;
	buffer += uitodec( buffer, sec, 2 );
}

//////////////////////////////////////////////////////////////////////////////
void printdate( Print *p, uint16_t year, uint8_t month, uint8_t day, uint8_t dow, uint8_t yeardigits, bool showdow, char datesep, char dowsep )
{
	char buffer[6], *bptr;
	if( yeardigits ) {
		bptr = buffer;
		if( yeardigits > 4 ) yeardigits = 4;
		bptr += uitodec( bptr, year, yeardigits );
		*bptr++ = datesep;
		*bptr = 0;
		p->print(buffer);
	}
	bptr = buffer;
	bptr += uitodec( bptr, month, 2 );
	*bptr++ = datesep;
	bptr += uitodec( bptr, day, 2 );
	*bptr = 0; p->print( buffer );
	if( showdow ) {
		bptr = buffer;
		*bptr++ = dowsep;
		bptr += uitodec( bptr, dow, 1);
		p->print( buffer );
	}
}

//////////////////////////////////////////////////////////////////////////////
void printtime( Print *p, uint8_t hour, uint8_t min, uint8_t sec, char sep )
{
	char buffer[4];
	uitodec( buffer, hour, 2 ); buffer[2] = sep; buffer[3] = 0; p->print( buffer );
	uitodec( buffer, min, 2 ); buffer[2] = sep; buffer[3] = 0; p->print( buffer );
	uitodec( buffer, sec, 2 ); buffer[2] = 0; p->print( buffer );
}

//////////////////////////////////////////////////////////////////////////////
bool parsedatetime( ts &t, const char *&inptr )
{
	//	"2015.10.28-3 16:37:05"
	t.year = getintparam( inptr );
	if( t.year == -1 ) return false;
	t.mon = getintparam( inptr );
	if( t.mon == -1 ) return false;
	t.mday = getintparam( inptr );
	if( t.mday == -1 ) return false;
	t.wday = getintparam( inptr );
	if( t.wday == -1 ) return false;
	t.hour = getintparam( inptr );
	if( t.hour == -1 ) return false;
	t.min = getintparam( inptr );
	if( t.min == -1 ) return false;
	t.sec = getintparam( inptr );
	if( t.sec == -1 ) t.sec = 0;
	return true;
}
