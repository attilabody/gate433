/*
 * Singleton.h
 *
 *  Created on: Jan 13, 2018
 *      Author: compi
 */

#ifndef SINGLETON_H_
#define SINGLETON_H_

template<typename T> class Singleton
{
protected:
	Singleton() {};

public:
	static T& Instance() {
		static T instance;
		return instance;
	}
};

#endif /* SINGLETON_H_ */
