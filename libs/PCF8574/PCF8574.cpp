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
//        renamed status() to lastError();
// 0.1.01 added value(); returns last read 8 bit value (cached);
//        value() does not always reflect the latest state of the pins!
// 0.1.00 initial version
//

#include "PCF8574.h"

#include <I2C.h>

PCF8574::PCF8574(uint8_t deviceAddress)
: _address(deviceAddress)
, _data(0xff)
, _error(0)
{
    // Wire.begin();
    // TWBR = 12; // 400KHz
}

// removed Wire.beginTransmission(addr);
// with  @100KHz -> 265 micros()
// without @100KHz -> 132 micros()
// without @400KHz -> 52 micros()
// TODO @800KHz -> ??
uint8_t PCF8574::read8( uint8_t &value)
{
	read8();
	value = _data; // last value
    return _error;
}

uint8_t PCF8574::read8()
{
	_error = I2c.read( _address, 1);
	if(!_error) {
	    _data = I2c.receive();
    }
    return _error;
}

uint8_t PCF8574::value()
{
    return _data;
}

uint8_t PCF8574::write8(uint8_t value)
{
#ifdef __PCF8574_VERBOSE
	Serial.print(F("PCF8574::write8: 0x"));
	Serial.println(value, HEX );
#endif	//	PCF8574_VERBOSE
    _data = value;
    _error = I2c.write(_address, value);
    return _error;
}

// pin should be 0..7
uint8_t PCF8574::read(uint8_t pin, uint8_t &value)
{
    PCF8574::read8();
    value = (_data & (1<<pin)) > 0;
    return _error;
}

// pin should be 0..7
uint8_t PCF8574::write(uint8_t pin, uint8_t value)
{
#ifdef __PCF8574_VERBOSE
	Serial.print(F("PCF8574::write: "));
	Serial.print(pin);
	Serial.print(F(", "));
	Serial.println(value );
#endif	//	PCF8574_VERBOSE

    if (value == LOW) _data &= ~(1<<pin);
    else _data |= (1<<pin);
    return PCF8574::write8(_data);
}

// pin should be 0..7
uint8_t PCF8574::toggle(uint8_t pin)
{
    _data ^=  (1 << pin);
    return PCF8574::write8(_data);
}

// n should be 0..7
uint8_t PCF8574::shiftRight(uint8_t n)
{
    if (n == 0 || n > 7 ) return 0xff;
    _data >>= n;
    return PCF8574::write8(_data);
}

// n should be 0..7
uint8_t PCF8574::shiftLeft(uint8_t n)
{
    if (n == 0 || n > 7) return 0xff;
    _data <<= n;
    return PCF8574::write8(_data);
}

int PCF8574::lastError()
{
    int e = _error;
    _error = 0;
    return e;
}
//
// END OF FILE
//
