/*
 * i2ceeprom.h
 *
 *  Created on: Jan 23, 2018
 *      Author: compi
 */

#ifndef INC_SG_I2CEEPROM_H_
#define INC_SG_I2CEEPROM_H_

#include <stdint.h>
#include <sg/stm32_hal.h>
#include <sg/i2cmaster.h>

namespace sg {

class I2cEEPROM
{
public:
	I2cEEPROM(I2cMaster &i2c, uint8_t i2cAddress, uint8_t addressLength, uint8_t pageLength, I2cMaster::Mode mode = I2cMaster::It);

	HAL_StatusTypeDef Read(void* _buffer, uint32_t address, uint32_t length);
	HAL_StatusTypeDef Write(const void* _buffer, uint32_t address, uint32_t length);
	uint32_t Sync() { return m_i2c.WaitCallback(); }

protected:
	HAL_StatusTypeDef PollStatus();

	I2cMaster			&m_i2c;
	uint16_t			m_i2cAddress;
	uint8_t				m_addressLengt;
	uint8_t				m_pageLength;
	uint8_t				m_needPoll = false;

	const I2cMaster::Mode m_mode;
};

}


#endif /* INC_SG_I2CEEPROM_H_ */
