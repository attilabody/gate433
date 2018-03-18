/*
 * Singleton.h
 *
 *  Created on: Jan 13, 2018
 *      Author: compi
 */

#ifndef SINGLETON_H_
#define SINGLETON_H_

#include <sg/ItLock.h>
namespace sg {

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

template<typename T> struct SafeSingletonInitializer
{
	SafeSingletonInitializer() {
		sg::ItLock	l;
		T::Instance();
	}
};
}	//	namespace sg

#endif /* SINGLETON_H_ */
