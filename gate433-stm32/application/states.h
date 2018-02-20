/*
 * states.h
 *
 *  Created on: Feb 18, 2018
 *      Author: compi
 */

#ifndef STATES_H_
#define STATES_H_

enum States
{
	OFF=0,
	CODEWAIT,
	CONFLICT,
	ACCEPT,
	WARN,
	DENY,
	UNREGISTERED,
	HURRY,
	NUMSTATES
};

#endif /* STATES_H_ */
