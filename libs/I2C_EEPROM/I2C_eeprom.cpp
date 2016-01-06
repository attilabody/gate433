#include <Wire.h> //I2C library

void i2c_eeprom_write_byte(int deviceaddress, unsigned int eeaddress, uint8_t data)
{
	int rdata = data;
	Wire.beginTransmission(deviceaddress);
	Wire.write((int) (eeaddress >> 8)); // MSB
	Wire.write((int) (eeaddress & 0xFF)); // LSB
	Wire.write(rdata);
	Wire.endTransmission();
}

// WARNING: address is a page address, 6-bit end will wrap around
// also, data can be maximum of about 30 bytes, because the Wire library has a buffer of 32 bytes
void i2c_eeprom_write_page(int deviceaddress, unsigned int eeaddresspage, uint8_t* data, uint8_t length)
{
	Wire.beginTransmission(deviceaddress);
	Wire.write((int) (eeaddresspage >> 8)); // MSB
	Wire.write((int) (eeaddresspage & 0xFF)); // LSB
	uint8_t c;
	for (c = 0; c < length; c++)
		Wire.write(data[c]);
	Wire.endTransmission();
}

uint8_t i2c_eeprom_read_byte(int deviceaddress, unsigned int eeaddress)
{
	uint8_t rdata = 0xFF;
	Wire.beginTransmission(deviceaddress);
	Wire.write((int) (eeaddress >> 8)); // MSB
	Wire.write((int) (eeaddress & 0xFF)); // LSB
	Wire.endTransmission();
	Wire.requestFrom(deviceaddress, 1);
	if (Wire.available())
		rdata = Wire.read();
	return rdata;
}

// maybe let's not read more than 30 or 32 bytes at a time!
void i2c_eeprom_read_buffer(int deviceaddress, unsigned int eeaddress, uint8_t *buffer, int length)
{
	Wire.beginTransmission(deviceaddress);
	Wire.write((int) (eeaddress >> 8)); // MSB
	Wire.write((int) (eeaddress & 0xFF)); // LSB
	Wire.endTransmission();
	Wire.requestFrom(deviceaddress, length);
	int c = 0;
	for (c = 0; c < length; c++)
		if (Wire.available())
			buffer[c] = Wire.read();
}
