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

#include <Wire.h>

#include <Wire.h> //I2C library

void i2c_eeprom_write_byte( int deviceaddress, unsigned int eeaddress, uint8_t data );
// WARNING: address is a page address, 6-bit end will wrap around
// also, data can be maximum of about 30 bytes, because the Wire library has a buffer of 32 bytes
void i2c_eeprom_write_page( int deviceaddress, unsigned int eeaddresspage, uint8_t* data, uint8_t length );
uint8_t i2c_eeprom_read_byte( int deviceaddress, unsigned int eeaddress );
// maybe let's not read more than 30 or 32 bytes at a time!
void i2c_eeprom_read_buffer( int deviceaddress, unsigned int eeaddress, uint8_t *buffer, int length );
#endif
// END OF FILE
