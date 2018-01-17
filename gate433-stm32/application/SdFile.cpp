/*
 * SdFile.cpp
 *
 *  Created on: Jan 17, 2018
 *      Author: abody
 */

#include <SdFile.h>
#include <fatfs.h>
#include <sd_diskio.h>

//////////////////////////////////////////////////////////////////////////////
bool SdFile::m_libInitialized = false;

//////////////////////////////////////////////////////////////////////////////
SdFile::SdFile()
{
	if(!m_libInitialized && !FATFS_LinkDriver(&SD_Driver, m_sdPath) && !f_mount(&m_fatFS, (TCHAR const*)m_sdPath, 0))
			m_libInitialized = true;
}

//////////////////////////////////////////////////////////////////////////////
SdFile::~SdFile()
{
	if(m_isOpen)
		f_close(&m_file);
}

//////////////////////////////////////////////////////////////////////////////
bool SdFile::Open(const char *name, SdFile::OpenMode mode)
{
	uint8_t	om;

	if(m_isOpen)
		return false;

	if(mode == OpenMode::READ)
		om = FA_READ;
	else if(mode == OpenMode::WRITE_TRUNC)
		om = FA_WRITE | FA_CREATE_ALWAYS;
	else if(mode == OpenMode::WRITE_APPEND)
		om = FA_WRITE | FA_OPEN_EXISTING;

	FRESULT ret = f_open(&m_file, name, om);

	if(ret == FR_OK)
		return true;

	m_lastError = ret;
	return false;
}

//////////////////////////////////////////////////////////////////////////////
bool SdFile::Close()
{
	if(!m_isOpen)
		return false;

	FRESULT ret = f_close(&m_file);
	if(ret == FR_OK)
		return true;

	m_lastError = ret;
	return false;
}

//////////////////////////////////////////////////////////////////////////////
bool SdFile::Seek(uint32_t pos)
{
	if(!m_isOpen)
		return false;

	FRESULT	ret = f_lseek(&m_file, pos);
	if(ret == FR_OK) {
		m_pos = pos;
		return true;
	}

	m_lastError = ret;
	return false;
}
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
