/*
 * display.cpp
 *
 *  Created on: Jan 30, 2016
 *      Author: compi
 */

#include <display.h>
#include <sg/Strutil.h>
#include <string.h>


//////////////////////////////////////////////////////////////////////////////
Display::Display(sg::I2cMaster &i2c, uint8_t i2cAddress)
: sg::I2cLcd(i2c, i2cAddress)
{
	sg::I2cLcd::Init();
	UpdateLoopStatus(false, false, false);

	uint8_t offset = 0;
	for(uint8_t line = 0; line < 4; ++line) {
		m_lineOffsets[line] = offset;
		offset += WIDTH;
	}
	memset(m_backBuffer, ' ', sizeof(m_backBuffer));
}

//////////////////////////////////////////////////////////////////////////////
void Display::Update(uint8_t x, uint8_t y, const char *str)
{
	m_x = x;
	m_y = y;
	m_needPos = true;
	Update(str);
}

//////////////////////////////////////////////////////////////////////////////
void Display::Update(const char *str)
{
	char *bufPtr = m_backBuffer + m_lineOffsets[m_y] + m_x;
	while(*str && m_x < WIDTH) {
		if(*bufPtr != *str) {
			if(m_needPos)
				SetCursor(m_x, m_y);
			*bufPtr = *str;
			Print(*str);
			m_needPos = false;
		} else
			m_needPos = true;
		++m_x;
		++str;
		++bufPtr;
	}
}

//////////////////////////////////////////////////////////////////////////////
void Display::UpdateDt(const sg::DS3231::Ts &dt, bool deSync)
{
	char	lcdBuffer[13];

	dt.TimeToString(lcdBuffer, 0, true);
	Update(0,1, lcdBuffer);

	dt.YMDToString(lcdBuffer, 2);
	Update(0, 0, lcdBuffer);

	Update(8, 0, deSync ? "!" : " ");

	m_dt = dt;
	m_deSync = deSync;
}

//////////////////////////////////////////////////////////////////////////////
void Display::UpdateLoopStatus( bool inner, bool outer, bool conflict )
{
	Update(9, 0, (outer ? "O" : (conflict ? "o" : "_")));
	Update(inner ? "I" : (conflict ? "i" : "_"));
}

//////////////////////////////////////////////////////////////////////////////
void Display::UpdateLastReceivedId( uint16_t id )
{
	if(m_lastReceivedId != id) {
		char buffer[5];
		sg::ToDec(buffer, id, 4, '0');
		Update(12, 0, buffer);
		m_lastReceivedId = id;
	}
}

//////////////////////////////////////////////////////////////////////////////
States Display::UpdateLastDecision(States state, uint16_t id, char reason)
{
	char buf[5] { (state == States::DENY && reason && reason != ' ') ? reason : g_stateSigns[state], ' ', 0 };

	Update(10, 1, buf);
	sg::ToDec(buf, id, 4, '0');
	Update(buf);
	return state;
}

