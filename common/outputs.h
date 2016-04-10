/*
 * outputs.h
 *
 *  Created on: Apr 10, 2016
 *      Author: compi
 */

#ifndef COMMON_OUTPUTS_H_
#define COMMON_OUTPUTS_H_

class outputs
{
public:
	virtual uint8_t	set(uint8_t pin, uint8_t value) = 0;
	virtual uint8_t	set(uint8_t value) = 0;
};




#endif /* COMMON_OUTPUTS_H_ */
