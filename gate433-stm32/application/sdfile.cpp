/*
 * SdFile.cpp
 *
 *  Created on: Jan 17, 2018
 *      Author: abody
 */

#include <fatfs.h>
#include <sd_diskio.h>
#include <sdfile.h>

//////////////////////////////////////////////////////////////////////////////
bool SdDriver::m_libInitialized = false;
FATFS SdDriver::m_fatFS;
char SdDriver::m_sdPath[4];

//////////////////////////////////////////////////////////////////////////////
SdDriver::SdDriver()
{
	if(!m_libInitialized && !FATFS_LinkDriver(&SD_Driver, m_sdPath) && !f_mount(&m_fatFS, (TCHAR const*)m_sdPath, 0))
			m_libInitialized = true;
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
SdFile::~SdFile()
{
	if(m_isOpen)
		f_close(&m_file);
}

//////////////////////////////////////////////////////////////////////////////
FRESULT SdFile::Open(const char *path, SdFile::OpenMode mode)
{
	if(m_isOpen && Close() != FR_OK)
		return FR_INT_ERR;

	FRESULT ret = f_open(&m_file, path, mode);

	if(ret == FR_OK) {
		m_isOpen = true;
		m_pos = 0;
	}
	return ret;
}

//////////////////////////////////////////////////////////////////////////////
FRESULT SdFile::Close()
{
	if(!m_isOpen)
		return FR_INT_ERR;

	FRESULT ret = f_close(&m_file);
	if(ret == FR_OK) {
		m_isOpen = false;
	}
	return ret;
}

//////////////////////////////////////////////////////////////////////////////
FRESULT SdFile::Seek(uint32_t pos)
{
	if(!m_isOpen)
		return FR_INT_ERR;

	FRESULT	ret = f_lseek(&m_file, pos);
	if(ret == FR_OK) {
		m_pos = pos;
		return ret;
	}

	return ret;
}
//////////////////////////////////////////////////////////////////////////////
FRESULT SdFile::Read(void *buffer, unsigned int size, unsigned int *read)
{
	if(!m_isOpen)
		return FR_INT_ERR;
	unsigned int _read = 0;
	FRESULT res = f_read(&m_file, buffer, size, &_read);
	if(res == FR_OK) {
		m_pos += _read;
		if(read)
			*read = _read;
	}
	return res;
}

//////////////////////////////////////////////////////////////////////////////
FRESULT SdFile::Write(void *buffer, unsigned int size, unsigned int *written)
{
	if(!m_isOpen)
		return FR_INT_ERR;
	unsigned int _written = 0;
	FRESULT ret = f_write(&m_file, buffer, size, &_written);
	if(ret == FR_OK) {
		m_pos += _written;
		if(written)
			*written = _written;
	}
	return ret;
}

//////////////////////////////////////////////////////////////////////////////
FRESULT SdFile::Sync()
{
	return m_isOpen ? f_sync(&m_file) : FR_INT_ERR;
}

//////////////////////////////////////////////////////////////////////////////
uint32_t SdFile::Size()
{
	return m_isOpen ? m_file.fsize : 0;
}

//////////////////////////////////////////////////////////////////////////////
