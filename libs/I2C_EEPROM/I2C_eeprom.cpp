#include <Arduino.h>
#include <I2C.h> //I2C library
#include "I2C_eeprom.h"
//#include "toolbox.h"

i2c_eeprom::i2c_eeprom( uint8_t i2caddress, uint8_t addressbits, uint8_t pagesize )
: m_i2caddress( i2caddress )
, m_addressbits( addressbits )
{
	m_pagesize = ( pagesize & (pagesize-1) ) ? 1 : pagesize;	//has to be 2^n
}

uint8_t i2c_eeprom::write_byte( eepromaddress_t eeaddress, uint8_t data)
{
	uint8_t  ret(I2c.write16(m_i2caddress, eeaddress, data));		//TODO: I2C error handling
	delay(5);
	return ret;
}

uint8_t i2c_eeprom::read_byte( eepromaddress_t eeaddress, uint8_t &result)
{
	uint8_t ret(I2c.read16(m_i2caddress, eeaddress, 3));
	if(I2c.available()) {	//TODO: I2C error handling
		result = I2c.receive();
	} else {
		ret = 0xff;
	}
	return ret;
}

uint8_t i2c_eeprom::write_page( eepromaddress_t eeaddress, uint8_t* data, eepromaddress_t length)
{
	eepromaddress_t nextpageaddress( (eeaddress & ~((eepromaddress_t)m_pagesize - 1)) + m_pagesize );
	uint8_t ret(0);
	while( length )
	{
		eepromaddress_t	remainingpagelen(nextpageaddress - eeaddress );
		if( remainingpagelen > length ) remainingpagelen = length;
		length -= remainingpagelen;
		while( remainingpagelen ) {
			if((ret = _write_page( eeaddress, data, remainingpagelen )) != 0)
				return ret;
			eeaddress += remainingpagelen;
			data += remainingpagelen;
			remainingpagelen -= remainingpagelen;
			delay(5);
		}
		nextpageaddress += m_pagesize;
	}
	return 0;
}

uint8_t i2c_eeprom::read_page( eepromaddress_t eeaddress, uint8_t* data, eepromaddress_t length)
{
	eepromaddress_t nextpageaddress( (eeaddress & ~((eepromaddress_t)m_pagesize - 1)) + m_pagesize );
	uint8_t ret(0);
	while( length )
	{
		eepromaddress_t	remainingpagelen(nextpageaddress - eeaddress );
		if( remainingpagelen > length ) remainingpagelen = length;
		length -= remainingpagelen;
		while( remainingpagelen ) {
			if((ret = _read_page( eeaddress, data, remainingpagelen )) != 0)
				return ret;
			eeaddress += remainingpagelen;
			data += remainingpagelen;
			remainingpagelen -= remainingpagelen;
			delay(5);
		}
		nextpageaddress += m_pagesize;
	}
	return 0;
}

uint8_t i2c_eeprom::fill_page( eepromaddress_t eeaddress, uint8_t data, eepromaddress_t length )
{
	eepromaddress_t nextpageaddress( (eeaddress & ~((eepromaddress_t)m_pagesize - 1)) + m_pagesize );
	uint8_t ret(0);
	while( length )
	{
		eepromaddress_t	remainingpagelen(nextpageaddress - eeaddress );
		if( remainingpagelen > length ) remainingpagelen = length;
		length -= remainingpagelen;
		while( remainingpagelen ) {
			if((ret = _fill_page( eeaddress, data, remainingpagelen )) != 0)
				return ret;
			eeaddress += remainingpagelen;
			remainingpagelen -= remainingpagelen;
			delay(5);
		}
		nextpageaddress += m_pagesize;
	}
	return 0;
}

uint8_t i2c_eeprom::_write_page( eepromaddress_t eeaddresspage, uint8_t* data, uint8_t length)
{
	return I2c.write16(m_i2caddress, eeaddresspage, data, length );
}

uint8_t i2c_eeprom::_fill_page( eepromaddress_t eeaddresspage, uint8_t data, uint8_t length)
{
	uint8_t ret(0);
	for (uint8_t c = 0; c < length; c++)
		if((ret = I2c.write16(m_i2caddress, eeaddresspage++, data)) != 0)
			return ret;
	return 0;
}

uint8_t i2c_eeprom::_read_page( eepromaddress_t eeaddresspage, uint8_t *buffer, uint8_t length)
{
	return I2c.read16(m_i2caddress, eeaddresspage, length, buffer);
}
