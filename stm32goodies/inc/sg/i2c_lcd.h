/*
 * i2c_lcd.h
 *
 *  Created on: Jun 20, 2016
 *      Author: compi
 */
#if !defined(I2C_LCD_H_)// && defined(HAVE_I2C) && defined(USE_I2C)
#define I2C_LCD_H_

#include <sg/stm32_hal.h>
#include <sg/i2cmaster.h>

namespace sg {

class I2cLcd
{
public:
	I2cLcd(I2cMaster &i2c, uint16_t i2cAddress);
	I2cLcd(const I2cLcd &rhs) = delete;
	I2cMaster::Status	Init();
	I2cMaster::Status	Clear();
	I2cMaster::Status	Home();
	I2cMaster::Status	SetCursor(uint8_t x, uint8_t y);
	I2cMaster::Status	Print(const char c) { return SendByte(c, false); }
	I2cMaster::Status	Print(const char* str);
	I2cMaster::Status	Print(uint32_t i, bool hex = false, uint8_t pad = 0, uint8_t *count = nullptr);

private:
	I2cMaster::Status SendData();
	I2cMaster::Status Epulse();
	I2cMaster::Status SendByte(uint8_t b, bool isCmd);
	I2cMaster::Status SendNibble(uint8_t nibble);

	I2cMaster			&m_i2c;
	uint8_t				m_data;
	uint16_t			m_i2cAddress;

	static const uint8_t m_rowOffsets[4];
	static const uint8_t m_init[5];
};

}	//namespace

#endif /* I2C_LCD_H_ */
