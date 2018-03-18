/*
 * stm32_hal.h
 *
 *  Created on: Nov 20, 2016
 *      Author: compi
 */

#ifndef INC_STM32_HAL_H_
#define INC_STM32_HAL_H_

#ifdef STM32F0
#include <stm32f0xx_hal.h>
#elif defined(STM32F1)
#include <stm32f1xx_hal.h>
#else
#error "Unknown STM32 MCU"
#endif

#endif /* INC_STM32_HAL_H_ */
