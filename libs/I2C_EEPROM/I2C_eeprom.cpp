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

void i2c_eeprom::write_byte( eepromaddress_t eeaddress, uint8_t data)
{
	I2c.write16(m_i2caddress, eeaddress, data);		//TODO: I2C error handling
	delay(5);
}

uint8_t i2c_eeprom::read_byte( eepromaddress_t eeaddress)
{
	I2c.read16(m_i2caddress, eeaddress, 3);
	if(I2c.available()) {	//TODO: I2C error handling
		return I2c.receive();
	} else {
		return 0;
	}
}

void i2c_eeprom::write_page( eepromaddress_t eeaddress, uint8_t* data, eepromaddress_t length)
{
	eepromaddress_t nextpageaddress( (eeaddress & ~((eepromaddress_t)m_pagesize - 1)) + m_pagesize );
	while( length )
	{
		eepromaddress_t	remainingpagelen(nextpageaddress - eeaddress );
		if( remainingpagelen > length ) remainingpagelen = length;
		length -= remainingpagelen;
		while( remainingpagelen ) {
			_write_page( eeaddress, data, remainingpagelen );
			eeaddress += remainingpagelen;
			data += remainingpagelen;
			remainingpagelen -= remainingpagelen;
			delay(5);
		}
		nextpageaddress += m_pagesize;
	}
}

void i2c_eeprom::read_page( eepromaddress_t eeaddress, uint8_t* data, eepromaddress_t length)
{
	eepromaddress_t nextpageaddress( (eeaddress & ~((eepromaddress_t)m_pagesize - 1)) + m_pagesize );
	while( length )
	{
		eepromaddress_t	remainingpagelen(nextpageaddress - eeaddress );
		if( remainingpagelen > length ) remainingpagelen = length;
		length -= remainingpagelen;
		while( remainingpagelen ) {
			_read_page( eeaddress, data, remainingpagelen );
			eeaddress += remainingpagelen;
			data += remainingpagelen;
			remainingpagelen -= remainingpagelen;
			delay(5);
		}
		nextpageaddress += m_pagesize;
	}
}

void i2c_eeprom::fill_page( eepromaddress_t eeaddress, uint8_t data, eepromaddress_t length )
{
	eepromaddress_t nextpageaddress( (eeaddress & ~((eepromaddress_t)m_pagesize - 1)) + m_pagesize );
	while( length )
	{
		eepromaddress_t	remainingpagelen(nextpageaddress - eeaddress );
		if( remainingpagelen > length ) remainingpagelen = length;
		length -= remainingpagelen;
		while( remainingpagelen ) {
			_fill_page( eeaddress, data, remainingpagelen );
			eeaddress += remainingpagelen;
			remainingpagelen -= remainingpagelen;
			delay(5);
		}
		nextpageaddress += m_pagesize;
	}
}

void i2c_eeprom::_write_page( eepromaddress_t eeaddresspage, uint8_t* data, uint8_t length)
{
	I2c.write16(m_i2caddress, eeaddresspage, data, length );
}

void i2c_eeprom::_fill_page( eepromaddress_t eeaddresspage, uint8_t data, uint8_t length)
{
	for (uint8_t c = 0; c < length; c++)
		I2c.write16(m_i2caddress, eeaddresspage++, data);
}

void i2c_eeprom::_read_page( eepromaddress_t eeaddresspage, uint8_t *buffer, uint8_t length)
{
	I2c.read16(m_i2caddress, eeaddresspage, length, buffer);
}
