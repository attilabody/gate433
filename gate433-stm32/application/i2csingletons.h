/*
 * i2csingletons.h
 *
 *  Created on: Apr 17, 2018
 *      Author: abody
 */

#ifndef I2CSINGLETONS_H_
#define I2CSINGLETONS_H_
#include <sg/Singleton.h>
#include <sg/I2cEeprom.h>
#include "i2c.h"

static uint8_t const	I2CEEPROMADDRESS = 0x50 << 1;
static uint8_t const 	I2CEEPROMADDRESSLENGTH = 2;
static uint8_t const	I2CEEPROMPAGELENGTH = 128;

class I2c1: public sg::I2cMaster, public sg::Singleton<I2c1>
{
	friend class sg::Singleton<I2c1>;
	I2c1() : sg::I2cMaster(&hi2c1, sg::I2cCallbackDispatcher::Instance()) {}
};

class MainI2cEeprom : public sg::I2cEEPROM,  public sg::Singleton<MainI2cEeprom>
{
	friend class sg::Singleton<MainI2cEeprom>;
	MainI2cEeprom() : sg::I2cEEPROM(I2c1::Instance(), I2CEEPROMADDRESS, I2CEEPROMADDRESSLENGTH, I2CEEPROMPAGELENGTH) {}
public:
};

#endif /* I2CSINGLETONS_H_ */
