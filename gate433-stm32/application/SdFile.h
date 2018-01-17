/*
 * SdFile.h
 *
 *  Created on: Jan 17, 2018
 *      Author: abody
 */

#ifndef SDFILE_H_
#define SDFILE_H_
#include <ff.h>

class SdFile {
public:
	SdFile();
	~SdFile();

	FRESULT	GetLastError() { return m_lastError; }
	enum class OpenMode {
		READ, WRITE_TRUNC, WRITE_APPEND
	};
	bool Open(const char *name, OpenMode mode);
	bool Close();

	uint32_t Read(void *buffer, uint32_t size);
	uint32_t Write(void *buffer, uint32_t size);
	bool Seek(uint32_t pos);

private:
	FIL				m_file;
	bool			m_isOpen = false;
	uint32_t		m_pos;
	FRESULT			m_lastError = FR_OK;

	static bool		m_libInitialized;
	static FATFS	m_fatFS;
	static char		m_sdPath[4];
};

#endif /* SDFILE_H_ */
