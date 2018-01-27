/*
 * dbgserial.h
 *
 *  Created on: Dec 6, 2016
 *      Author: compi
 */

#ifndef USART_H
#define USART_H

#include <sg/stm32_hal.h>
#include <sg/singleton.h>
#include <stddef.h>
#include <string.h>
#include <inttypes.h>

namespace sg {

extern "C" {
	void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);
	void HAL_UART_TxHalfCpltCallback(UART_HandleTypeDef *huart);
	void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
	void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart);
	void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart);
}

class UsartCallbackDispatcher : public Singleton<UsartCallbackDispatcher>
{
private:
	static const uint8_t	USART_COUNT = 3;
public:
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

	IUsartCallback 					*m_handlers[USART_COUNT];

	friend void ::HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);
	friend void ::HAL_UART_TxHalfCpltCallback(UART_HandleTypeDef *huart);
	friend void ::HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
	friend void ::HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart);
	friend void ::HAL_UART_ErrorCallback(UART_HandleTypeDef *huart);
	friend class Singleton<UsartCallbackDispatcher>;
};

////////////////////////////////////////////////////////////////////
class DbgUsart : public UsartCallbackDispatcher::IUsartCallback, public Singleton<DbgUsart>
{
	friend class Singleton<DbgUsart>;
public:
	DbgUsart(const DbgUsart &) = delete;
	DbgUsart(DbgUsart &&) = delete;
	DbgUsart &operator=(const DbgUsart &) = delete;

	bool Init(
			UsartCallbackDispatcher &disp,
			UART_HandleTypeDef *huart,
			uint8_t *outputBuffer,
			uint16_t outputBufferSize,
			bool block);
	void SetBlock(bool block) { m_block = block; }

	uint16_t Send(const void *buffer, uint16_t count);
	uint16_t Send(char c);
	uint16_t Send(bool b);
	uint16_t Send(uint32_t u, bool hex = false, bool prefix = true, bool pad = false);
	uint16_t Send(uint16_t u, bool hex = false, bool prefix = true, bool pad = false);
	uint16_t Send(uint8_t u, bool hex = false, bool prefix = true);
	uint16_t Send(const char *str);

	class Hex {};
	class Dec {};
	class Prefix {};
	class Noprefix {};
	class Pad {};
	class Nopad {};
	struct Buffer {
		Buffer(void* buffer, uint16_t count) : m_buffer(buffer), m_count(count) {}
		void *m_buffer;
		uint16_t m_count;
	};

	DbgUsart& operator<<(const Buffer& b) { Send(b.m_buffer, b.m_count); return *this; }
	template<typename T> DbgUsart& operator<<(T* ptr) { Send(reinterpret_cast<const char*>(ptr)); return *this; }
	DbgUsart& operator<<(char c) { Send(&c, 1); return *this; }
	DbgUsart& operator<<(bool b) { Send(b ? '1' : '0'); return *this; }
	DbgUsart& operator<<(uint32_t u);
	DbgUsart& operator<<(uint16_t u);
	DbgUsart& operator<<(uint8_t u);

	DbgUsart& operator<<(Hex) { m_hex = true; return *this; }
	DbgUsart& operator<<(Dec) { m_hex = false; return *this; }
	DbgUsart& operator<<(Prefix) { m_prefix = true; return *this; }
	DbgUsart& operator<<(Noprefix) { m_prefix = false; return *this; }
	DbgUsart& operator<<(Pad) { m_pad = true; return *this; }
	DbgUsart& operator<<(Nopad) { m_pad = false; return *this; }

	struct IReceiverCallback {
		virtual void LineReceived(uint16_t count) = 0;
	};
	HAL_StatusTypeDef Receive(
			uint8_t *buffer,
			uint16_t bufferSize,
			IReceiverCallback *callback,
			void *callbackUserPtr = nullptr);

private:
	DbgUsart() = default;
	HAL_StatusTypeDef FillTxBuffer(const uint8_t *buffer, uint16_t &count);
	virtual bool UsartCallback(UART_HandleTypeDef *huart, CallbackType type);

	UART_HandleTypeDef	*m_huart = nullptr;
	uint8_t				*m_outputBuffer = nullptr;
	uint16_t			m_outputBufferSize = 0;
	uint8_t				*m_inputBuffer = nullptr;
	uint16_t			m_inputBufferSize = 0;
	IReceiverCallback	*m_receiverCallback = nullptr;
	void				*m_receivedCallbackUserPtr = nullptr;
	bool				m_block = true;
	bool				m_prefix = false;
	bool				m_hex = false;
	bool				m_pad = false;

	volatile uint16_t	m_txStart = 0;
	volatile uint16_t	m_txCount = 0;
	volatile uint16_t	m_chunkSize = 0;
};

}	// namespace sg

#endif /* DBGSERIAL_H_ */
