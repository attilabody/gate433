/*
 * SdFile.h
 *
 *  Created on: Jan 17, 2018
 *      Author: abody
 */

#ifndef SDFILE_H_
#define SDFILE_H_
#include <ff.h>

class SdDriver
{
protected:
	SdDriver();
	FRESULT	GetLastError() { return m_lastError; }
	bool HandleRet(FRESULT ret) {
		if(ret == FR_OK)
			return true;
		m_lastError = ret;
		return false;
	}

	FRESULT			m_lastError = FR_OK;

	static bool		m_libInitialized;
	static FATFS	m_fatFS;
	static char		m_sdPath[4];
};

class SdVolume : public SdDriver
{
public:
	bool MkDir(const char *path);
	bool Unlink(const char *path);
	bool Rename(const char *oldPath, const char *newPath);
	bool Stat(FILINFO &info, const char *path);
private:
};

class SdFile : public SdDriver
{
public:
	~SdFile();

	enum class OpenMode {
		READ, WRITE_TRUNC, WRITE_APPEND, RW_TRUNC, RW_APPEND
	};
	bool Open(const char *path, OpenMode mode);
	bool Close();

	unsigned int Read(void *buffer, unsigned int size);
	unsigned int Write(void *buffer, unsigned int size);
	bool Sync();
	bool Seek(uint32_t pos);
	uint32_t Ftell() { return m_pos; }

private:
	FIL				m_file;
	bool			m_isOpen = false;
	uint32_t		m_pos = 0;
};

#endif /* SDFILE_H_ */
