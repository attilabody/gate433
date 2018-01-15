/*
 * dbgserial.cpp
 *
 *  Created on: Dec 6, 2016
 *      Author: compi
 */

#include <sg/usart.h>
#include <stdlib.h>
#include <sg/itlock.h>
#include <sg/strutil.h>

using namespace sg;

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	UsartCallbackDispatcher::Instance().Callback(huart, UsartCallbackDispatcher::IUsartCallback::TxCpltCallback);
}

void HAL_UART_TxHalfCpltCallback(UART_HandleTypeDef *huart)
{
	UsartCallbackDispatcher::Instance().Callback(huart, UsartCallbackDispatcher::IUsartCallback::TxHalfCpltCallback);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	UsartCallbackDispatcher::Instance().Callback(huart, UsartCallbackDispatcher::IUsartCallback::RxCpltCallback);
}
void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
	UsartCallbackDispatcher::Instance().Callback(huart, UsartCallbackDispatcher::IUsartCallback::RxHalfCpltCallback);
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	UsartCallbackDispatcher::Instance().Callback(huart, UsartCallbackDispatcher::IUsartCallback::ErrorCallback);
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
bool UsartCallbackDispatcher::Register(IUsartCallback *handler)
{
	decltype(handler)*	hp = nullptr;

	for( auto& h : m_handlers) {
		if( h == handler )
			return true;
		if(!hp && !h) {
			hp = &h;
			break;
		}
	}
	if(hp) {
		*hp = handler;
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////
void UsartCallbackDispatcher::Callback(UART_HandleTypeDef *huart, IUsartCallback::CallbackType type)
{
	for(auto& handler : m_handlers) {
		if(handler && handler->UsartCallback(huart, type))
			break;
	}
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
bool DbgUsart::Init(UsartCallbackDispatcher &disp, UART_HandleTypeDef *huart, uint8_t *buffer, uint16_t size, bool block)
{
	if(m_buffer) {
		return false;
	}
	m_huart = huart;
	m_buffer = buffer;
	m_size = size;
	m_block = block;

	disp.Register(this);

	return true;
}

////////////////////////////////////////////////////////////////////
uint16_t DbgUsart::FillTxBuffer(const uint8_t *buffer, uint16_t count)
{
	HAL_StatusTypeDef	st;
	uint16_t   			free, freestart, tocopy, copied = 0;

	{
		ItLock	lock;
		freestart = m_txStart + m_txCount;
		free = m_size - m_txCount;
	}
	freestart -= (freestart >= m_size) ? m_size : 0;
	if(count > free) count = free;
	tocopy = freestart + count > m_size ? m_size - freestart : count;

	memcpy(m_buffer + freestart, buffer, tocopy);

	{
		ItLock	lock;
		if(!m_txCount) {
			lock.Release();
			m_chunkSize = tocopy;
			st = HAL_UART_Transmit_IT(m_huart, m_buffer + freestart, tocopy);
		}
		copied = tocopy;
		count -= tocopy;
		lock.Acquire();
		m_txCount += tocopy;
	}

	if(!count)
		return copied;

	buffer += tocopy;
	memcpy(m_buffer, buffer, count);

	bool schedule;
	{
		ItLock lock;
		schedule = !m_txCount;
		m_txCount += count;
	}
	if(schedule)	//	unlikely corner case
		st = HAL_UART_Transmit_IT(m_huart, m_buffer, count);

	return copied + count;
}

////////////////////////////////////////////////////////////////////
bool DbgUsart::UsartCallback(UART_HandleTypeDef *huart, CallbackType type)
{
	HAL_StatusTypeDef	st;

	if(huart != m_huart)
		return false;
	if(m_txCount && type == CallbackType::TxCpltCallback)
	{
		m_txCount -= m_chunkSize;
		m_txStart += m_chunkSize;
		if(m_txStart >= m_size)
			m_txStart -= m_size;
		m_chunkSize = m_txStart + m_txCount > m_size ? m_size - m_txStart : m_txCount;
		if(m_chunkSize)
			st = HAL_UART_Transmit_IT(m_huart, m_buffer + m_txStart, m_chunkSize);
	}
	return true;
}

////////////////////////////////////////////////////////////////////
uint16_t  DbgUsart::Send(const void *buffer, uint16_t count)
{
	uint16_t  sent = 0, copied;

	while(count) {
		while(m_txCount == m_size)
			if(!m_block)
				return sent;

		copied = FillTxBuffer((uint8_t*)buffer, count);
		buffer = (uint8_t*)buffer + copied;
		count -= copied;
		sent += copied;
	}

	return sent;
}

////////////////////////////////////////////////////////////////////
uint16_t  DbgUsart::Send(char c)
{
	return Send(&c, 1);
}


////////////////////////////////////////////////////////////////////
uint16_t DbgUsart::Send(bool b)
{
	return Send(b ? '1' : '0');
}


////////////////////////////////////////////////////////////////////
uint16_t DbgUsart::Send(uint32_t u, bool hex, bool prefix, bool zeroes)
{
	if(hex) {
		uint16_t ret = 0;
		if(prefix)
			ret += Send("0x");
		uint8_t	*pb = ((uint8_t *)((&u) + 1) - 1);
		uint8_t b;
		for(uint8_t byte = 0; byte < sizeof(u); ++byte) {
			b = *pb >> 4;
			if(zeroes || b)
				ret += Send(tochr(b));
			if(b) zeroes = true;
			b = *pb & 0xF;
			if(zeroes || b || !byte)
				ret += Send(tochr(b));
			--pb;
		}
		return ret;
	} else {
		char buffer[13];
		uitodec(u, buffer);
		return Send(buffer, strlen(buffer));
	}
}


////////////////////////////////////////////////////////////////////
uint16_t DbgUsart::Send(uint16_t u, bool hex, bool prefix, bool zeroes)
{
	if(hex) {
		uint16_t  ret = 0;
		if(prefix)
			ret += Send("0x");
		uint8_t	*pb = ((uint8_t *)((&u) + 1) - 1);
		uint8_t b;
		for(uint8_t byte = 0; byte < sizeof(u); ++byte) {
			b = *pb >> 4;
			if(zeroes || b)
				ret += Send(tochr(b));
			if(b) zeroes = true;
			b = *pb & 0xF;
			if(zeroes || b || !byte)
				ret += Send(tochr(b));
			--pb;
		}
		return ret;
	} else {
		char    buffer[8];
		uitodec(u, buffer);
		return Send(buffer, strlen(buffer));
	}
}


////////////////////////////////////////////////////////////////////
uint16_t DbgUsart::Send(uint8_t u, bool hex, bool prefix)
{
	if(hex) {
		uint16_t  ret = 0;
		if(prefix)
			ret += Send("0x");
		ret += Send(tochr(u >> 4));
		ret += Send(tochr(u & 0x0F));
		return ret;
	} else {
		char    buffer[4];
		uitodec(u, buffer);
		return Send(buffer, strlen(buffer));
	}
}


////////////////////////////////////////////////////////////////////
uint16_t DbgUsart::Send(const char *str)
{
	return Send((void *)str, strlen(str));
}



