/*
 * display.cpp
 *
 *  Created on: Jan 30, 2016
 *      Author: compi
 */

#include <display.h>
//#include "toolbox.h"
//#include "dthelpers.h"

//////////////////////////////////////////////////////////////////////////////
Display::Display(sg::I2cMaster &i2c, uint8_t i2cAddress, uint8_t width, uint8_t height)
: sg::I2cLcd(i2c, i2cAddress)
, m_width(width)
, m_height(height)
{
}

//////////////////////////////////////////////////////////////////////////////
Display::~Display()
{
}

//////////////////////////////////////////////////////////////////////////////
void Display::UpdateDt(const sg::DS3231::Ts &dt, bool deSync)
{
	char	lcdbuffer[13];
	sg::DS3231::Ts	tmp = dt;

	if(dt.sec != m_dt.sec || dt.min != m_dt.min || dt.hour != m_dt.hour) {
		SetCursor(0,1);
		dt.TimeToString(lcdbuffer, 0, true);
		Print(lcdbuffer);
	}

	if(dt.year != m_dt.year || dt.mon != m_dt.mon || dt.mday != m_dt.mday) {
		SetCursor(0,0);
		dt.YMDToString(lcdbuffer, 2);
		Print(lcdbuffer);
	}

	if(deSync != m_deSync) {
		SetCursor(8,0);
		Print(deSync ? "!" : " ");
	}
	m_dt = dt;
	m_deSync = deSync;
}

//////////////////////////////////////////////////////////////////////////////
void Display::UpdateLoopStatus( bool inner, bool outer, bool conflict )
{
	SetCursor(9, 0);
	Print(outer ? "O" : (conflict ? "o" : "_"));
	Print(inner ? "I" : (conflict ? "i" : "_"));
}

//////////////////////////////////////////////////////////////////////////////
void Display::UpdateLastReceivedId( uint16_t id )
{
	if(m_lastReceivedId != id) {
		SetCursor(12, 0);
		Print(id, false, 4);
		m_lastReceivedId = id;
	}
}

//////////////////////////////////////////////////////////////////////////////
void Display::UpdateLastDecision( char decision, uint16_t id )
{
	char buf[3] { decision, ' ', 0 };

	buf[4] = 0;
	SetCursor(10, 1);
	Print(buf);
	Print(id, false, 4);
}

