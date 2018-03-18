/*
 * dbgserial.h
 *
 *  Created on: Dec 6, 2016
 *      Author: compi
 */

#ifndef USART_H
#define USART_H

#include <sg/Stm32Hal.h>
#include <sg/Singleton.h>
#include <stddef.h>
#include <string.h>
#include <inttypes.h>
#include <sg/Strutil.h>

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
	virtual ~Usart() = default;

	void SetBlock(bool block) { m_block = block; }
	bool GetAndClearError();

	size_t Send(const void *buffer, uint16_t count);
	size_t Send(char c);
	size_t Send(bool b);
	size_t Send(const char *str);
	size_t Send(char *str) { return Send(static_cast<const char*>(str)); }

	template<typename T> size_t SendInt(T u, bool hex = false, bool prefix = true, uint8_t digits = 0)
	{
		char buffer[digits ? digits + 2 : sizeof(u) * 3 + 1];
		char *bptr = buffer;
		size_t ret = 0;

		if(hex) {
			if(prefix)
				ret = Send("0x", 2);
			return Send(buffer, ToHex(buffer, u, digits)) + ret;
		} else {
			if(u < 0) {
				*bptr++ = '-';
				u = 0 - u;
			}
			return Send(buffer, ToDec(bptr, u, digits) + (bptr - buffer));
		}
	}

	struct Hex {};
	struct Dec {};
	struct Prefix {};
	struct NoPrefix {};
	struct NoPad {};
	struct Endl {};
	struct Pad { uint8_t w; Pad(uint8_t _w) : w(_w) {} };

	static const Hex		hex;
	static const Dec		dec;
	static const Prefix		prefix;
	static const NoPrefix	noprefix;
	static const NoPad		nopad;
	static const Endl		endl;

	struct Buffer {
		Buffer(const void* buffer, uint16_t count) : m_buffer(buffer), m_count(count) {}
		const void *m_buffer;
		uint16_t m_count;
	};

	Usart& operator<<(const Buffer& b) { Send(b.m_buffer, b.m_count); return *this; }
	template<typename T> Usart& operator<<(T* ptr) { Send(static_cast<const char*>(ptr)); return *this; }
	Usart& operator<<(char c) { Send(&c, 1); return *this; }
	Usart& operator<<(bool b) { Send(b ? '1' : '0'); return *this; }

	Usart& operator<<(Hex) { m_hex = true; return *this; }
	Usart& operator<<(Dec) { m_hex = false; return *this; }
	Usart& operator<<(Prefix) { m_prefix = true; return *this; }
	Usart& operator<<(NoPrefix) { m_prefix = false; return *this; }
	Usart& operator<<(const Pad &p) { m_digits = p.w; return *this; }
	Usart& operator<<(NoPad) { m_digits = 0; return *this; }
	Usart& operator<<(Endl) { Send("\r\n", 2); return *this; }

	template<typename T> Usart& operator<<(T u) { SendInt(u, m_hex, m_prefix, m_digits); return *this; }

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
	bool				m_errorOccured = false;
	bool				m_block = true;
	bool				m_prefix = false;
	bool				m_hex = false;
	uint8_t				m_digits = 0;

	volatile uint16_t	m_txStart = 0;
	volatile uint16_t	m_txCount = 0;
	//volatile uint16_t	m_chunkSize = 0;
};

}	// namespace sg

#endif /* DBGSERIAL_H_ */
