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
bool SdDriver::m_libInitialized = false;
FATFS SdDriver::m_fatFS;
char SdDriver::m_sdPath[4];

SdDriver::SdDriver()
{
	if(!m_libInitialized && !FATFS_LinkDriver(&SD_Driver, m_sdPath) && !f_mount(&m_fatFS, (TCHAR const*)m_sdPath, 0))
			m_libInitialized = true;
}

//////////////////////////////////////////////////////////////////////////////
bool SdVolume::Unlink(const char *path)
{
	return HandleRet(f_unlink(path));
}

//////////////////////////////////////////////////////////////////////////////
bool SdVolume::MkDir(const char *path)
{
	return HandleRet(f_mkdir(path));
}

//////////////////////////////////////////////////////////////////////////////
bool SdVolume::Rename(const char *oldPath, const char *newPath)
{
	return HandleRet(f_rename(oldPath, newPath));
}

bool SdVolume::Stat(FILINFO &info, const char *path)
{
	return HandleRet(f_stat(path, &info));
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
bool SdFile::Open(const char *path, SdFile::OpenMode mode)
{
	if(m_isOpen && !Close())
		return false;

	FRESULT ret = f_open(&m_file, path, mode);

	if(ret == FR_OK) {
		m_isOpen = true;
		m_pos = 0;
		return true;
	}
	m_lastError = ret;
	return false;
}

//////////////////////////////////////////////////////////////////////////////
bool SdFile::Close()
{
	if(!m_isOpen)
		return false;

	FRESULT ret = f_close(&m_file);
	if(ret == FR_OK) {
		m_isOpen = false;
		return true;
	}

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
unsigned int SdFile::Read(void *buffer, unsigned int size)
{
	if(!m_isOpen)
		return 0;
	unsigned int read = 0;
	FRESULT res = f_read(&m_file, buffer, size, &read);
	if(res == FR_OK) {
		m_pos += read;
		return read;
	}
	m_lastError = res;
	return 0;
}

//////////////////////////////////////////////////////////////////////////////
unsigned int SdFile::Write(void *buffer, unsigned int size)
{
	if(!m_isOpen)
		return 0;
	unsigned int written = 0;
	FRESULT ret = f_write(&m_file, buffer, size, &written);
	if(ret == FR_OK) {
		m_pos += written;
		return written;
	}
	m_lastError = ret;
	return 0;
}

//////////////////////////////////////////////////////////////////////////////
bool SdFile::Sync()
{
	return m_isOpen ? HandleRet(f_sync(&m_file)) : false;
}

//////////////////////////////////////////////////////////////////////////////
uint32_t SdFile::Size()
{
	return m_isOpen ? m_file.fsize : 0;
}

//////////////////////////////////////////////////////////////////////////////
