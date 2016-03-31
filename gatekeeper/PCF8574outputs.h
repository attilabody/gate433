/*
 * PCF8574outputs.h
 *
 *  Created on: Mar 31, 2016
 *      Author: compi
 */

#ifndef PCF8574OUTPUTS_H_
#define PCF8574OUTPUTS_H_

#include <PCF8574.h>

class PCF8574outputs: private PCF8574
{
public:
	PCF8574outputs(uint8_t i2caddress);
	uint8_t write(uint8_t pin, uint8_t value);
	uint8_t write8(uint8_t value);
};

#endif /* PCF8574OUTPUTS_H_ */
