/*
 * ITick.h
 *
 *  Created on: Feb 2, 2018
 *      Author: compi
 */

#ifndef ITICK_H_
#define ITICK_H_

class ITick {
public:
	virtual void Tick(uint32_t now) = 0;
};

#endif /* ITICK_H_ */
