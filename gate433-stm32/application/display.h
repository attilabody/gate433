/*
 * display.h
 *
 *  Created on: Jan 30, 2016
 *      Author: compi
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_
#include <sg/i2c_lcd.h>
#include <sg/ds3231.h>

struct ts;

class Display : protected sg::I2cLcd
{
public:
	Display(sg::I2cMaster &i2c, uint8_t i2c_address, uint8_t width, uint8_t height);
	~Display();
	using sg::I2cLcd::Init;

	void UpdateDt(const sg::DS3231::Ts &dt, bool deSync);
	void UpdateLoopStatus( bool inner, bool outer, bool conflict );
	void UpdateLastReceivedId( uint16_t id );
	void UpdateLastDecision( char decision, uint16_t id );

	enum CHANGETYPE {
		DATECHANGED = 1,
		TIMECHANGED = 2,
		SYNCCHANGED = 4
	};

protected:
	uint8_t	m_width;
	uint8_t	m_height;
	sg::DS3231::Ts	m_dt;
	bool			m_deSync = false;
	uint16_t		m_lastReceivedId = 0;
};

#endif /* DISPLAY_H_ */
