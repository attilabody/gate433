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

	static bool		m_libInitialized;
	static FATFS	m_fatFS;
	static char		m_sdPath[4];
};

class SdVolume : public SdDriver
{
public:
	FRESULT MkDir(const char *path) { return f_mkdir(path); }
	FRESULT Unlink(const char *path) { return f_unlink(path); }
	FRESULT Rename(const char *oldPath, const char *newPath) { return f_rename(oldPath, newPath); }
	FRESULT Stat(FILINFO &info, const char *path) { return f_stat(path, &info); }
private:
};

class SdFile : public SdDriver
{
public:
	~SdFile();

	enum OpenMode : uint8_t {
		READ = FA_READ,
		OPEN_EXISTING = FA_OPEN_EXISTING,
		WRITE = FA_WRITE,
		CREATE_NEW = FA_CREATE_NEW,
		CREATE_ALWAYS = FA_CREATE_ALWAYS,
		OPEN_ALWAYS = FA_OPEN_ALWAYS
	};
	FRESULT Open(const char *path, OpenMode mode);
	FRESULT Close();

	FRESULT Read(void *buffer, unsigned int size, unsigned int *read = nullptr);
	FRESULT Write(void *buffer, unsigned int size, unsigned int *written = nullptr);
	FRESULT Sync();
	FRESULT Seek(uint32_t pos);
	uint32_t Size();
	uint32_t Ftell() { return m_pos; }
	bool IsOpen() { return m_isOpen; }

private:
	FIL				m_file;
	bool			m_isOpen = false;
	uint32_t		m_pos = 0;
};

#endif /* SDFILE_H_ */
