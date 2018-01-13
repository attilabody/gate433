/*
 * itblocker.h
 *
 *  Created on: Dec 7, 2016
 *      Author: compi
 */

#ifndef ITLOCK_H_
#define ITLOCK_H_

#include <sg/stm32_hal.h>

namespace sg {

class ItLock
{
public:
	inline ItLock();
	inline ~ItLock();
	inline void Enable();
	inline void Release();
	inline void Acquire();
private:
	bool m_originalPrimask;
};

inline ItLock::ItLock()
: m_originalPrimask(__get_PRIMASK() != 0)
{
	__disable_irq();
}

inline ItLock::~ItLock()
{
	Release();
}

inline void ItLock::Release()
{
	bool currentPrimask = __get_PRIMASK() != 0;
	if(currentPrimask && !m_originalPrimask) {
		__enable_irq();
	} else if(m_originalPrimask && !currentPrimask) {
		__disable_irq();
	}
}

inline void ItLock::Acquire()
{
	__disable_irq();
}

inline void ItLock::Enable()
{
	__enable_irq();
}

}	//	namespace sg

#endif /* STM32GOODIES_INC_ITLOCK_H_ */
