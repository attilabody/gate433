/*
 * i2c_lcd.c
 *
 *  Created on: Jun 20, 2016
 *      Author: compi
 */

//#include "config.h"
//#if defined(HAVE_I2C) && defined(USE_I2C)
#include <sg/stm32_hal.h>
//#include <util/delay.h>

#include "sg/i2c_lcd.h"
#include "sg/strutil.h"

using namespace sg;

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// flags for backlight control
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

#define En 0b00000100  // Enable bit
#define Rw 0b00000010  // Read/Write bit
#define Rs 0b00000001  // Register select bit

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
const uint8_t I2cLcd::m_rowOffsets[4] =  { 0x00, 0x40, 0x14, 0x54 };

const uint8_t I2cLcd::m_init[5] = {
	0x28,
	0x01,	//clear display
	0x06,	//increment mode, no display shift
	0x0C,	//display on, hide cursor, not blinking
};

//////////////////////////////////////////////////////////////////////////////
I2cLcd::I2cLcd(I2cMaster &i2c, uint16_t i2cAddress)
: m_i2c(i2c)
, m_data(LCD_BACKLIGHT)
, m_i2cAddress(i2cAddress)
{
}

//////////////////////////////////////////////////////////////////////////////
// 200 us @ 100kHz
inline I2cMaster::Status I2cLcd::SendData()
{
	return m_i2c.Write(m_i2cAddress, &m_data, sizeof(m_data));
}

//////////////////////////////////////////////////////////////////////////////
// 400 us @ 100kHz
inline I2cMaster::Status I2cLcd::Epulse()
{
	I2cMaster::Status	ret;
	m_data |= En;
	ret = SendData();
	m_data &= ~En;
	if(ret == HAL_OK)
		return SendData();
	return ret;
}

//////////////////////////////////////////////////////////////////////////////
// 600 us @ 100kHz
inline I2cMaster::Status I2cLcd::SendNibble(uint8_t nibble)
{
	I2cMaster::Status ret;
	m_data = ((m_data & 0x0f) | (nibble & 0xf0));
	ret = SendData();
	if(ret == HAL_OK)
		return Epulse();
	return ret;
}

//////////////////////////////////////////////////////////////////////////////
inline I2cMaster::Status I2cLcd::SendByte(uint8_t b, bool isCmd)
{
	I2cMaster::Status ret;

	m_data = ((m_data & 0x0f & ~Rs) | (b & 0xf0) | (isCmd ? 0 : Rs));
	ret = SendData();
	if(ret == HAL_OK) {
		ret = Epulse();
		if(ret == HAL_OK) {
			m_data = ((m_data & 0x0f & ~Rs) | ((b & 0x0f) << 4) | (isCmd ? 0 : Rs));
			ret = SendData();
			if(ret == HAL_OK)
				return Epulse();
		}
	}
	return ret;
}

//////////////////////////////////////////////////////////////////////////////
I2cMaster::Status I2cLcd::Init()
{
	I2cMaster::Status ret;
	uint8_t count;

	m_data = LCD_BACKLIGHT;
	ret = SendData();
	if(ret == HAL_OK) {
		HAL_Delay(500);
		ret = SendNibble(0x30);
		if(ret == HAL_OK) {
			HAL_Delay(5);
			ret = SendNibble(0x30);
			if(ret == HAL_OK) {
				ret = SendNibble(0x30);
				if(ret == HAL_OK) {
					ret = SendNibble(0x20);
					if(ret == HAL_OK)
						for (count = 0; count < sizeof(m_init); count++) {
							ret = SendByte(m_init[count], true);
							if(ret != HAL_OK)
								return ret;
							HAL_Delay(3);
						}
	}}}}
	return ret;
}

//////////////////////////////////////////////////////////////////////////////
I2cMaster::Status I2cLcd::Clear()
{
	I2cMaster::Status ret = SendByte(LCD_CLEARDISPLAY, true);	// clear display, set cursor position to zero
	HAL_Delay(3);												// this command takes a long time!
	return ret;
}

//////////////////////////////////////////////////////////////////////////////
I2cMaster::Status I2cLcd::Home()
{
	I2cMaster::Status ret = SendByte(LCD_RETURNHOME, true);		// set cursor position to zero
	HAL_Delay(2);												// this command takes a long time!
	return ret;
}

//////////////////////////////////////////////////////////////////////////////
I2cMaster::Status I2cLcd::SetCursor(uint8_t x, uint8_t y)
{
	return SendByte(LCD_SETDDRAMADDR | (x + m_rowOffsets[y & 3]), true);
}

//////////////////////////////////////////////////////////////////////////////
I2cMaster::Status I2cLcd::Print(const char *str)
{
	I2cMaster::Status ret = HAL_OK;
	while(*str) {
		ret = SendByte(*str++, false);
		if(ret != HAL_OK)
			return ret;
	}
	return ret;
}

//////////////////////////////////////////////////////////////////////////////
I2cMaster::Status I2cLcd::Print(uint32_t u, bool hex, uint8_t pad, uint8_t *count)
{
	char	buffer[11];
	I2cMaster::Status	ret = HAL_OK;

	if(pad > 10) pad = 10;

	size_t _count = hex ? tohex(buffer, u, pad, '0') : todec(buffer, u, pad, '0');
	if((ret = Print(buffer)) != HAL_OK)
		return ret;

	if(count)
		*count = _count > pad ? _count : pad;
	return ret;
}


//#endif	//	#if defined(HAVE_I2C) && defined(USE_I2C)
