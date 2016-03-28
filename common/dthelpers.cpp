/*
 * dthelpers.cpp
 *
 *  Created on: Feb 8, 2016
 *      Author: abody
 */
#include <Arduino.h>
#include "dthelpers.h"
#include "toolbox.h"
#include <ds3231.h>

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
	//	"2015.10.28 16:37:05"
	t.year = getintparam( inptr );
	if( t.year == -1 ) return false;
	t.mon = getintparam( inptr );
	if( t.mon == -1 ) return false;
	t.mday = getintparam( inptr );
	if( t.mday == -1 ) return false;
	t.hour = getintparam( inptr );
	if( t.hour == -1 ) return false;
	t.min = getintparam( inptr );
	if( t.min == -1 ) return false;
	t.sec = getintparam( inptr );
	if( t.sec == -1 ) t.sec = 0;
	return true;
}
