/*
 * ITick.h
 *
 *  Created on: Feb 2, 2018
 *      Author: compi
 */

#ifndef TICK_H_
#define TICK_H_
#include <inttypes.h>

class ITick {
public:
	virtual void Tick(uint32_t now) = 0;
};

#endif /* TICK_H_ */
