#include <Arduino.h>
#include <Wire.h> //I2C library
#include "I2C_eeprom.h"
//#include "toolbox.h"

#define TWI_MAX_TRANSFER_SIZE	30

i2c_eeprom::i2c_eeprom( uint8_t i2caddress, uint8_t addressbits, uint8_t pagesize )
: m_i2caddress( i2caddress )
, m_addressbits( addressbits )
{
	m_pagesize = ( pagesize & (pagesize-1) ) ? 1 : pagesize;	//has to be 2^n
}

void i2c_eeprom::write_byte( eepromaddress_t eeaddress, uint8_t data)
{
	int rdata = data;
	Wire.beginTransmission(m_i2caddress);
	Wire.write((int) (eeaddress >> 8)); // MSB
	Wire.write((int) (eeaddress & 0xFF)); // LSB
	Wire.write(rdata);
	Wire.endTransmission();
	delay(5);
}

uint8_t i2c_eeprom::read_byte( eepromaddress_t eeaddress)
{
	uint8_t rdata = 0xFF;
	Wire.beginTransmission(m_i2caddress);
	Wire.write((int) (eeaddress >> 8)); // MSB
	Wire.write((int) (eeaddress & 0xFF)); // LSB
	Wire.endTransmission();
	Wire.requestFrom((int)m_i2caddress, 1);
	if (Wire.available())
		rdata = Wire.read();
	return rdata;
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
			uint8_t curlen = remainingpagelen > TWI_MAX_TRANSFER_SIZE ? TWI_MAX_TRANSFER_SIZE : remainingpagelen;
			_write_page( eeaddress, data, curlen );
			eeaddress += curlen;
			data += curlen;
			remainingpagelen -= curlen;
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
			uint8_t curlen = remainingpagelen > TWI_MAX_TRANSFER_SIZE ? TWI_MAX_TRANSFER_SIZE : remainingpagelen;
			_read_page( eeaddress, data, curlen );
			eeaddress += curlen;
			data += curlen;
			remainingpagelen -= curlen;
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
			uint8_t curlen = remainingpagelen > TWI_MAX_TRANSFER_SIZE ? TWI_MAX_TRANSFER_SIZE : remainingpagelen;
			_fill_page( eeaddress, data, curlen );
			eeaddress += curlen;
			remainingpagelen -= curlen;
			delay(5);
		}
		nextpageaddress += m_pagesize;
	}
}

void i2c_eeprom::_write_page( eepromaddress_t eeaddresspage, uint8_t* data, uint8_t length)
{
	Wire.beginTransmission(m_i2caddress);
	Wire.write((int) (eeaddresspage >> 8)); // MSB
	Wire.write((int) (eeaddresspage & 0xFF)); // LSB
	uint8_t c;
	for (c = 0; c < length; c++)
		Wire.write(data[c]);
	Wire.endTransmission();
}

void i2c_eeprom::_fill_page( eepromaddress_t eeaddresspage, uint8_t data, uint8_t length)
{
	Wire.beginTransmission(m_i2caddress);
	Wire.write((int) (eeaddresspage >> 8)); // MSB
	Wire.write((int) (eeaddresspage & 0xFF)); // LSB
	uint8_t c;
	for (c = 0; c < length; c++)
		Wire.write(data);
	Wire.endTransmission();
}

void i2c_eeprom::_read_page( eepromaddress_t eeaddresspage, uint8_t *buffer, uint8_t length)
{
	Wire.beginTransmission(m_i2caddress);
	Wire.write((int) (eeaddresspage >> 8)); // MSB
	Wire.write((int) (eeaddresspage & 0xFF)); // LSB
	Wire.endTransmission();
	Wire.requestFrom((int)m_i2caddress, (int)length);
	int pos = 0;
	for (pos = 0; pos < length; pos++)
		if (Wire.available())
			buffer[pos] = Wire.read();
}
