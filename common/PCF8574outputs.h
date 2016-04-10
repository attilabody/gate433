/*
 * PCF8574outputs.h
 *
 *  Created on: Mar 31, 2016
 *      Author: compi
 */

#ifndef PCF8574OUTPUTS_H_
#define PCF8574OUTPUTS_H_

#include <PCF8574.h>
#include <outputs.h>

class PCF8574outputs: private PCF8574, public outputs
{
public:
	PCF8574outputs(uint8_t i2caddress);

	virtual uint8_t set(uint8_t pin, uint8_t value) { return PCF8574::write(pin, value); }
	virtual uint8_t set(uint8_t value) { return PCF8574::write8(value); }
};

#endif /* PCF8574OUTPUTS_H_ */
