//
//    FILE: PCF8574.cpp
//  AUTHOR: Rob Tillaart
//    DATE: 02-febr-2013
// VERSION: 0.1.04
// PURPOSE: I2C PCF8574 library for Arduino
//     URL:
//
// HISTORY:
// 0.1.04 2015-05-09 removed ambiguity in read8()
// 0.1.03 2015-03-02 address int -> uint8_t
// 0.1.02 replaced ints with uint8_t to reduce footprint;
//        added default value for shiftLeft() and shiftRight()
//        renamed hal_status() to lastError();
// 0.1.01 added value(); returns last read 8 bit value (cached);
//        value() does not always reflect the latest state of the pins!
// 0.1.00 initial version
//
//#include "config.h"
//#if defined(HAVE_I2C) && defined(USE_I2C)
#include "sg/pcf8574.h"

using namespace sg;

Pcf8574::Pcf8574(I2cMaster &i2c, uint8_t i2cAddress, uint8_t initialData)
: m_i2c(i2c)
, m_i2cAddress(i2cAddress)
, m_data(initialData)
{
}

I2cMaster::Status Pcf8574::Write(uint8_t value)
{
	m_data = value;
	return m_i2c.Write(m_i2cAddress, &m_data, sizeof(m_data), m_mode);
}

I2cMaster::Status Pcf8574::Write(uint8_t pin, bool value)
{
	if(!value)
		m_data &= ~(1 << pin);
	else
		m_data |= (1 << pin);

	return m_i2c.Write(m_i2cAddress, &m_data, sizeof(m_data), m_mode);
}

I2cMaster::Status Pcf8574::Read(uint8_t &value)
{
	I2cMaster::Status ret = m_i2c.Read(m_i2cAddress, &m_data, sizeof(m_data));
	value = m_data;
	return ret;
}

I2cMaster::Status Pcf8574::Read(uint8_t pin, bool &value)
{
	I2cMaster::Status ret = m_i2c.Read(m_i2cAddress, &m_data, sizeof(m_data));
	value = (m_data & (1 << pin)) != 0;
	return ret;
}

I2cMaster::Status Pcf8574::Toggle(uint8_t pin)
{
	m_data ^= (1 << pin);
	return m_i2c.Write(m_i2cAddress, &m_data, sizeof(m_data), m_mode);
}

I2cMaster::Status Pcf8574::ShiftRight(uint8_t n)
{
	m_data >>= n;
	return m_i2c.Write(m_i2cAddress, &m_data, sizeof(m_data), m_mode);
}

I2cMaster::Status Pcf8574::ShiftLeft(uint8_t n)
{
	m_data <<= n;
	return m_i2c.Write(m_i2cAddress, &m_data, sizeof(m_data), m_mode);
}

//#endif	//	#if defined(HAVE_I2C) && defined(USE_I2C)
//
// END OF FILE
//
