/*
 * display.h
 *
 *  Created on: Jan 30, 2016
 *      Author: compi
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_
#include <sg/Ds3231.h>
#include <sg/I2cLcd.h>
#include "states.h"

struct ts;

class Display : protected sg::I2cLcd
{
public:
	Display(sg::I2cMaster &i2c, uint8_t i2c_address);
	~Display() = default;
	using sg::I2cLcd::Init;

	void UpdateDt(const sg::DS3231::Ts &dt, bool deSync);
	void UpdateLoopStatus( bool inner, bool outer, bool conflict);
	void UpdateLastReceivedId( uint16_t id);
	States UpdateLastDecision(States state, uint16_t id, char reason);

	uint16_t GetLastReceivedId() { return m_lastReceivedId; }

protected:
	void Update(uint8_t x, uint8_t y, const char *str);
	void Update(const char *str);

	sg::DS3231::Ts	m_dt;
	bool			m_deSync = false;
	uint16_t		m_lastReceivedId = 0xffff;

	static const uint8_t WIDTH = 16;
	static const uint8_t HEIGHT = 2;
	uint8_t	m_x = 0, m_y = 0;
	bool	m_needPos = true;

	char	m_backBuffer[WIDTH * HEIGHT];
	uint8_t	m_lineOffsets[HEIGHT];
};

#endif /* DISPLAY_H_ */
