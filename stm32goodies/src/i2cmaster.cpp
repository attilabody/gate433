/*
 * i2cmaster.cpp
 *
 *  Created on: Nov 20, 2016
 *      Author: compi
 */

#include <sg/i2cmaster.h>
using namespace sg;

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	I2cCallbackDispatcher::Instance().Callback(hi2c, I2cCallbackDispatcher::II2cCallback::MasterTxCpltCallback);
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	I2cCallbackDispatcher::Instance().Callback(hi2c, I2cCallbackDispatcher::II2cCallback::MasterRxCpltCallback);
}

void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	I2cCallbackDispatcher::Instance().Callback(hi2c, I2cCallbackDispatcher::II2cCallback::SlaveTxCpltCallback);
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	I2cCallbackDispatcher::Instance().Callback(hi2c, I2cCallbackDispatcher::II2cCallback::SlaveRxCpltCallback);
}

void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	I2cCallbackDispatcher::Instance().Callback(hi2c, I2cCallbackDispatcher::II2cCallback::MemTxCpltCallback);
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	I2cCallbackDispatcher::Instance().Callback(hi2c, I2cCallbackDispatcher::II2cCallback::MemRxCpltCallback);
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
	I2cCallbackDispatcher::Instance().Callback(hi2c, I2cCallbackDispatcher::II2cCallback::ErrorCallback);
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
bool I2cCallbackDispatcher::Register(II2cCallback *handler)
{
	decltype(handler)*	hp = nullptr;

	for( auto& h : m_handlers) {
		if( h == handler )
			return true;
		if(!hp && !h)
			hp = &h;
	}
	if(hp) {
		*hp = handler;
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////
void I2cCallbackDispatcher::Callback(I2C_HandleTypeDef *hi2c, II2cCallback::CallbackType type)
{
	for(auto& handler : m_handlers) {
		if(handler && handler->I2cCallback(hi2c, type))
			break;
	}
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
I2cMaster::I2cMaster(I2C_HandleTypeDef *hi2c, I2cCallbackDispatcher &disp, Mode defaultMode)
: m_hi2c(hi2c)
, m_defautMode(defaultMode)
{
	disp.Register(this);
	if(m_defautMode == Default)
		m_defautMode = Poll;
}

//////////////////////////////////////////////////////////////////////////////
bool I2cMaster::I2cCallback(I2C_HandleTypeDef *hi2c, CallbackType type)
{
	if(hi2c != m_hi2c)
		return false;

	if(m_expectedCallback == type || type == ErrorCallback ) {
		m_expectedCallback = None;
		m_callbackError = type == ErrorCallback ? HAL_I2C_GetError(hi2c) : HAL_I2C_ERROR_NONE;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////
I2cMaster::Status I2cMaster::Write(const uint16_t i2cAddress, const void *data, uint8_t size, Mode mode)
{
	if(WaitCallback() != HAL_I2C_ERROR_NONE)
		return HAL_ERROR;
	if(mode == Default)
		mode = m_defautMode;
	if(mode == Poll) {
		return HAL_I2C_Master_Transmit(m_hi2c, i2cAddress, reinterpret_cast<uint8_t*>(const_cast<void*>(data)), size, HAL_MAX_DELAY);
	} else {
		Status ret;
		if(mode == It) {
			ret = HAL_I2C_Master_Transmit_IT(m_hi2c, i2cAddress, reinterpret_cast<uint8_t*>(const_cast<void*>(data)), size);
		} else {
			ret = HAL_I2C_Master_Transmit_DMA(m_hi2c, i2cAddress, reinterpret_cast<uint8_t*>(const_cast<void*>(data)), size);
		}
		if(ret == HAL_OK) {
			m_expectedCallback = MasterTxCpltCallback;
		}
		return ret;
	}
}

//////////////////////////////////////////////////////////////////////////////
I2cMaster::Status I2cMaster::Read(const uint16_t i2cAddress, void *data, uint8_t size, Mode mode)
{
	if(WaitCallback() != HAL_I2C_ERROR_NONE)
		return HAL_ERROR;
	if(mode == Default)
		mode = m_defautMode;
	if(mode == Poll) {
		return HAL_I2C_Master_Receive(m_hi2c, i2cAddress, (uint8_t*)data, size, HAL_MAX_DELAY);
	} else {
		Status ret;
		if(mode == It) {
			ret = HAL_I2C_Master_Receive_IT(m_hi2c, i2cAddress, (uint8_t*)data, size);
		} else {
			ret = HAL_I2C_Master_Receive_DMA(m_hi2c, i2cAddress, (uint8_t*)data, size);
		}
		if(ret == HAL_OK) {
			m_expectedCallback = MasterRxCpltCallback;
		}
		return ret;
	}
}

//////////////////////////////////////////////////////////////////////////////
I2cMaster::Status I2cMaster::WriteMem(const uint16_t i2cAddress, uint16_t memAddr, uint8_t memAddrSize, const void *data, uint16_t size, Mode mode)
{
	if(WaitCallback() != HAL_I2C_ERROR_NONE)
		return HAL_ERROR;
	if(mode == Default)
		mode = m_defautMode;
	if(mode == Poll) {
		return HAL_I2C_Mem_Write(m_hi2c, i2cAddress, memAddr, memAddrSize, reinterpret_cast<uint8_t*>(const_cast<void*>(data)), size, HAL_MAX_DELAY);
	} else {
		Status ret;
		if(mode == It) {
			ret = HAL_I2C_Mem_Write_IT(m_hi2c, i2cAddress, memAddr, memAddrSize, reinterpret_cast<uint8_t*>(const_cast<void*>(data)), size);
		} else {
			ret = HAL_I2C_Mem_Write_DMA(m_hi2c, i2cAddress, memAddr, memAddrSize, reinterpret_cast<uint8_t*>(const_cast<void*>(data)), size);
		}
		if(ret == HAL_OK) {
			m_expectedCallback = MemTxCpltCallback;
		}
		return ret;
	}
}

//////////////////////////////////////////////////////////////////////////////
I2cMaster::Status I2cMaster::ReadMem(const uint16_t i2cAddress, uint16_t memAddr, uint8_t memAddrSize, void *data, uint16_t size, Mode mode)
{
	if(WaitCallback() != HAL_I2C_ERROR_NONE)
		return HAL_ERROR;
	if(mode == Default)
		mode = m_defautMode;
	if(mode == Poll) {
		return HAL_I2C_Mem_Read(m_hi2c, i2cAddress, memAddr, memAddrSize, (uint8_t*)data, size, HAL_MAX_DELAY);
	} else {
		Status ret;
		if(mode == It) {
			ret = HAL_I2C_Mem_Read_IT(m_hi2c, i2cAddress, memAddr, memAddrSize, (uint8_t*)data, size);
		} else {
			ret = HAL_I2C_Mem_Read_DMA(m_hi2c, i2cAddress, memAddr, memAddrSize, (uint8_t*)data, size);
		}
		if(ret == HAL_OK) {
			m_expectedCallback = MemRxCpltCallback;
		}
		return ret;
	}
}
