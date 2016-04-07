/*
 * display.cpp
 *
 *  Created on: Jan 30, 2016
 *      Author: compi
 */

#include <display.h>
#include <ds3231.h>
#include "toolbox.h"
#include "dthelpers.h"

display::display( byte i2c_address, byte width, byte height )
: LiquidCrystal_I2C( i2c_address, width, height )
{
}

display::~display()
{
}

//////////////////////////////////////////////////////////////////////////////
void display::init()
{
	LiquidCrystal_I2C::init();
	backlight();
}

//////////////////////////////////////////////////////////////////////////////
void display::updatedt( const ts &dt, byte updatemask )
{
	char	lcdbuffer[13], *lbp( lcdbuffer );

	if( updatemask & 0x38 ) {
		setCursor(0,0);
		datetostring( lbp, dt.year, dt.mon, dt.mday, dt.wday, 0, true, '.', '/' ); *lbp = 0;
		print( lcdbuffer );
	}

	if( updatemask & 7 ) {
		setCursor(0,1);
		lbp = lcdbuffer;
		timetostring( lbp, dt.hour, dt.min, dt.sec, ':' ); *lbp++ = 0;
		print( lcdbuffer);
	}

}

//////////////////////////////////////////////////////////////////////////////
void display::updateloopstatus( bool inner, bool outer )
{
	setCursor( 9, 0 );
	print( inner ? '*' : '_' );
	print( outer ? '*' : '_' );
}

//////////////////////////////////////////////////////////////////////////////
void display::updatelastreceivedid( uint16_t id )
{
	char buf[5];

	uitodec( buf, id, 4);
	buf[4] = 0;
	setCursor( 12, 0 );
	print( buf );
}

//////////////////////////////////////////////////////////////////////////////
void display::updatelastdecision( char decision, uint16_t id )
{
	char buf[5];

	uitodec( buf, id, 4);
	buf[4] = 0;
	setCursor( 10, 1 );
	print( decision );
	print( ' ' );
	print( buf );
}

