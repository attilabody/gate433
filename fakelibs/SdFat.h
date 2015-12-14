/*
 * SdFat.h
 *
 *  Created on: Nov 8, 2015
 *      Author: compi
 */

#ifndef SDFAT_H_
#define SDFAT_H_

#define SS 10
#define SPI_HALF_SPEED 2

struct File
{
	bool isOpen() { return true; }
	operator bool() { return isOpen(); }
	bool close() { return true; }
	bool seek(uint32_t pos) { return true; }
	int read(void* buf, size_t nbyte) { return nbyte; }
	int write(const void* buf, size_t nbyte) { return nbyte; }
};

uint8_t const O_READ = 0X01;
uint8_t const O_RDONLY = O_READ;
uint8_t const O_WRITE = 0X02;
uint8_t const O_WRONLY = O_WRITE;
uint8_t const O_RDWR = (O_READ | O_WRITE);
uint8_t const O_ACCMODE = (O_READ | O_WRITE);
uint8_t const O_APPEND = 0X04;
uint8_t const O_SYNC = 0X08;
uint8_t const O_TRUNC = 0X10;
uint8_t const O_AT_END = 0X20;
uint8_t const O_CREAT = 0X40;
uint8_t const O_EXCL = 0X80;

#define FILE_READ O_READ
#define FILE_WRITE (O_RDWR | O_CREAT | O_AT_END)

struct SdFat
{
	bool begin(uint8_t csPin = SS, uint8_t divisor = 2) { return true; }
	File open(const char *path, uint8_t mode = FILE_READ) { File f; return f; }
};




#endif /* SDFAT_H_ */
