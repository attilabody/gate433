#ifndef I2C_EEPROM_H
#define I2C_EEPROM_H
//
//    FILE: I2C_eeprom.h
//  AUTHOR: Rob Tillaart
// PURPOSE: I2C_eeprom library for Arduino with EEPROM 24LC256 et al.
// VERSION: 1.2.03
// HISTORY: See I2C_eeprom.cpp
//     URL: http://arduino.cc/playground/Main/LibraryForI2CEEPROM
//
// Released to the public domain
//
#include <I2C.h> //I2C library

class i2c_eeprom
{
public:
	typedef uint16_t	eepromaddress_t;
	i2c_eeprom( uint8_t i2caddress, uint8_t addresbits, uint8_t pagesize );

	void write_byte( eepromaddress_t eeaddress, uint8_t data );
	uint8_t read_byte( eepromaddress_t eeaddress );
	void write_page( eepromaddress_t eeaddress, uint8_t* data, eepromaddress_t length );
	void read_page( eepromaddress_t eeaddress, uint8_t* data, eepromaddress_t length);
	void fill_page( eepromaddress_t eeaddresspage, uint8_t data, eepromaddress_t length );

protected:
	void _write_page( eepromaddress_t eeaddresspage, uint8_t* data, uint8_t length );
	void _fill_page( eepromaddress_t eeaddresspage, uint8_t data, uint8_t length );
	void _read_page( eepromaddress_t eeaddress, uint8_t *buffer, uint8_t length );
private:
	uint8_t m_i2caddress;
	uint8_t m_addressbits;
	uint8_t	m_pagesize;
};
#endif
// END OF FILE
