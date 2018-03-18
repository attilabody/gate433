/*
 * i2ceeprom.cpp
 *
 *  Created on: Jan 23, 2018
 *      Author: compi
 */
#include <sg/I2cEeprom.h>

namespace sg
{
I2cEEPROM::I2cEEPROM(I2cMaster &i2c, uint8_t i2cAddress, uint8_t addressLength, uint8_t pageLength, I2cMaster::Mode mode)
: m_i2c(i2c)
, m_i2cAddress(i2cAddress)
, m_addressLengt(addressLength)
, m_pageLength(pageLength)
, m_mode(mode)
{}

//////////////////////////////////////////////////////////////////////////////
HAL_StatusTypeDef I2cEEPROM::PollStatus()
{
	HAL_StatusTypeDef	ret;
	uint32_t			start = HAL_GetTick();

	while((ret = m_i2c.Write(m_i2cAddress, NULL, 0)) != HAL_I2C_ERROR_NONE) {
		if(HAL_GetTick() - start >=100)
			return ret;
		HAL_Delay(1);
	}
	m_needPoll = false;
	return ret;
}


//////////////////////////////////////////////////////////////////////////////

HAL_StatusTypeDef I2cEEPROM::Read(void* _buffer, uint32_t address, uint32_t length)
{
	HAL_StatusTypeDef	ret = HAL_OK;
	uint16_t			toRead;
	uint8_t				*buffer = (uint8_t*)_buffer;

	if(m_needPoll) {
		ret = PollStatus();
		if(ret != HAL_OK)
			return ret;
	}

	toRead = length > UINT16_MAX ? UINT16_MAX : length;

	while(length && ret == HAL_OK)
	{
		ret = m_i2c.ReadMem( m_i2cAddress, address, m_addressLengt, buffer, toRead, m_mode);
		length -= toRead;
		buffer += toRead;
		address += toRead;
		toRead = length > UINT16_MAX ? UINT16_MAX : length;
	}
	return ret;
}


//////////////////////////////////////////////////////////////////////////////
HAL_StatusTypeDef I2cEEPROM::Write(const void* _buffer, uint32_t address, uint32_t length)
{
	HAL_StatusTypeDef	ret = HAL_OK;
	uint8_t				toWrite;
	uint8_t				*buffer = (uint8_t*)_buffer;

	toWrite  = m_pageLength - (address & (m_pageLength -1));
	if(toWrite > length)
		toWrite = length;

	while(length && ret == HAL_OK)
	{
		if(m_needPoll) {
			ret = PollStatus();
			if(ret != HAL_OK)
				return ret;
		}
		if((ret = m_i2c.WriteMem(m_i2cAddress, address, m_addressLengt, buffer, toWrite, m_mode)) != HAL_OK) {
			return ret;
		}
		m_needPoll = 1;
		length -= toWrite;
		buffer += toWrite;
		address += toWrite;
		toWrite = length < m_pageLength ? length : m_pageLength;
	}
	return ret;
}


//////////////////////////////////////////////////////////////////////////////

}



