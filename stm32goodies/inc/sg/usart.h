/*
 * dbgserial.h
 *
 *  Created on: Dec 6, 2016
 *      Author: compi
 */

#ifndef USART_H
#define USART_H

#include <sg/stm32_hal.h>
#include <stddef.h>
#include <string.h>
#include <inttypes.h>

namespace sg {

#define USART_COUNT 3

extern "C" {
	void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);
	void HAL_UART_TxHalfCpltCallback(UART_HandleTypeDef *huart);
	void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
	void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart);
	void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart);
}

class UsartCallbackDispatcher
{
public:
	static UsartCallbackDispatcher& Instance() { return m_instance; }

	class IUsartCallback
	{
	public:
		enum CallbackType
		{
			None,
			TxCpltCallback,
			TxHalfCpltCallback,
			RxCpltCallback,
			RxHalfCpltCallback,
			ErrorCallback
		};
	private:
		virtual bool UsartCallback(UART_HandleTypeDef *huart, CallbackType type) = 0;
		friend class UsartCallbackDispatcher;
	};
	bool Register(IUsartCallback *handler);

private:
	UsartCallbackDispatcher() { memset(&m_handlers, 0, sizeof(m_handlers)); }
	UsartCallbackDispatcher(const UsartCallbackDispatcher &rhs) = delete;

	void Callback(UART_HandleTypeDef *hi2c, IUsartCallback::CallbackType type);

	static UsartCallbackDispatcher	m_instance;

	IUsartCallback 					*m_handlers[USART_COUNT];

	friend void ::HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);
	friend void ::HAL_UART_TxHalfCpltCallback(UART_HandleTypeDef *huart);
	friend void ::HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
	friend void ::HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart);
	friend void ::HAL_UART_ErrorCallback(UART_HandleTypeDef *huart);

};

////////////////////////////////////////////////////////////////////
class DbgUsart : public UsartCallbackDispatcher::IUsartCallback
{
public:
	DbgUsart(const DbgUsart &) = delete;
	DbgUsart(DbgUsart &&) = delete;
	DbgUsart &operator=(const DbgUsart &) = delete;

	bool Init(UsartCallbackDispatcher &disp, UART_HandleTypeDef *huart, uint8_t *buffer, uint16_t size, bool block);
	static DbgUsart& Instance() { return m_instance; }
	void SetBlock(bool block) { m_block = block; }

	uint16_t Send(const void *buffer, uint16_t count);
	uint16_t Send(char c);
	uint16_t Send(bool b);
	uint16_t Send(uint32_t l, bool hex = false, bool prefix = true, bool zeroes = false);
	uint16_t Send(uint16_t i, bool hex = false, bool prefix = true, bool zeroes = false);
	uint16_t Send(uint8_t i, bool hex = false, bool prefix = true);
	uint16_t Send(const char *str);

private:
	DbgUsart() = default;
	uint16_t FillTxBuffer(const uint8_t *buffer, uint16_t count);
	virtual bool UsartCallback(UART_HandleTypeDef *huart, CallbackType type);

	static DbgUsart		m_instance;

	UART_HandleTypeDef	*m_huart;
	uint8_t				*m_buffer = nullptr;
	uint16_t			m_size = 0;
	bool				m_block = true;

	volatile uint16_t	m_txStart = 0;
	volatile uint16_t	m_txCount = 0;
	volatile uint16_t	m_chunkSize = 0;
};

}	// namespace sg

#endif /* DBGSERIAL_H_ */
