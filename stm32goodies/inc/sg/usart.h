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
	static const uint8_t	MAX_USART_COUNT = 3;
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
	bool Register(IUsartCallback &handler);

private:
	UsartCallbackDispatcher() { memset(&m_handlers, 0, sizeof(m_handlers)); }
	UsartCallbackDispatcher(const UsartCallbackDispatcher &rhs) = delete;

	void Callback(UART_HandleTypeDef *hi2c, IUsartCallback::CallbackType type);

	IUsartCallback	*m_handlers[MAX_USART_COUNT];

	friend void ::HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);
	friend void ::HAL_UART_TxHalfCpltCallback(UART_HandleTypeDef *huart);
	friend void ::HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
	friend void ::HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart);
	friend void ::HAL_UART_ErrorCallback(UART_HandleTypeDef *huart);
	friend class Singleton<UsartCallbackDispatcher>;
};

////////////////////////////////////////////////////////////////////
class Usart : public UsartCallbackDispatcher::IUsartCallback
{
public:
	Usart(const Usart &) = delete;
	Usart(Usart &&) = delete;
	Usart &operator=(const Usart &) = delete;
	Usart(UART_HandleTypeDef *huart, UsartCallbackDispatcher &disp, char *outputBuffer, uint16_t outputBufferSize, bool block);

	void SetBlock(bool block) { m_block = block; }

	uint16_t Send(const void *buffer, uint16_t count);
	uint16_t Send(char c);
	uint16_t Send(bool b);
	uint16_t Send(uint32_t u, bool hex = false, bool prefix = true, bool pad = false);
	uint16_t Send(uint16_t u, bool hex = false, bool prefix = true, bool pad = false);
	uint16_t Send(uint8_t u, bool hex = false, bool prefix = true);
	uint16_t Send(const char *str);

	struct Hex {};
	struct Dec {};
	struct Prefix {};
	struct NoPrefix {};
	struct Pad {};
	struct NoPad {};
	struct Endl {};

	static const Hex		hex;
	static const Dec		dec;
	static const Prefix		prefix;
	static const NoPrefix	noprefix;
	static const Pad		pad;
	static const NoPad		nopad;
	static const Endl		endl;

	struct Buffer {
		Buffer(const void* buffer, uint16_t count) : m_buffer(buffer), m_count(count) {}
		const void *m_buffer;
		uint16_t m_count;
	};

	Usart& operator<<(const Buffer& b) { Send(b.m_buffer, b.m_count); return *this; }
	template<typename T> Usart& operator<<(T* ptr) { Send(reinterpret_cast<const char*>(ptr)); return *this; }
	Usart& operator<<(char c) { Send(&c, 1); return *this; }
	Usart& operator<<(bool b) { Send(b ? '1' : '0'); return *this; }
	Usart& operator<<(uint32_t u) { Send(u, m_hex, m_prefix, m_pad); return *this; }
	Usart& operator<<(uint16_t u) { Send(u, m_hex, m_prefix, m_pad); return *this; }
	Usart& operator<<(uint8_t u) { Send(u, m_hex, m_prefix); return *this; }

	Usart& operator<<(Hex) { m_hex = true; return *this; }
	Usart& operator<<(Dec) { m_hex = false; return *this; }
	Usart& operator<<(Prefix) { m_prefix = true; return *this; }
	Usart& operator<<(NoPrefix) { m_prefix = false; return *this; }
	Usart& operator<<(Pad) { m_pad = true; return *this; }
	Usart& operator<<(NoPad) { m_pad = false; return *this; }
	Usart& operator<<(Endl) { Send("\r\n", 2); return *this; }

	struct IReceiverCallback {
		virtual void LineReceived(char *buffer, uint16_t count) = 0;
	};
	HAL_StatusTypeDef Receive(
			char *buffer,
			uint16_t bufferSize,
			IReceiverCallback &callback,
			void *callbackUserPtr = nullptr);

private:
	Usart() = default;
	HAL_StatusTypeDef FillTxBuffer(const uint8_t *buffer, uint16_t &count);
	virtual bool UsartCallback(UART_HandleTypeDef *huart, CallbackType type);

	UART_HandleTypeDef	*m_huart = nullptr;
	char				*m_outputBuffer = nullptr;
	uint16_t			m_outputBufferSize = 0;
	char				*m_inputBuffer = nullptr;
	uint16_t			m_inputBufferSize = 0;
	IReceiverCallback	*m_receiverCallback = nullptr;
	void				*m_receivedCallbackUserPtr = nullptr;
	bool				m_block = true;
	bool				m_prefix = false;
	bool				m_hex = false;
	bool				m_pad = false;

	volatile uint16_t	m_txStart = 0;
	volatile uint16_t	m_txCount = 0;
	//volatile uint16_t	m_chunkSize = 0;
};

}	// namespace sg

#endif /* DBGSERIAL_H_ */
