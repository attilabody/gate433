/*
 * sdfatlogwriter.h
 *
 *  Created on: Jan 26, 2016
 *      Author: abody
 */

#ifndef SDFATLOGWRITER_H_
#define SDFATLOGWRITER_H_

#include <Arduino.h>
#include <SdFat.h>
#include "logwriter.h"
#include "writebuffer.h"

class SdFat;
class SdFile;
class FatFile;
struct ts;

class sdfatlogwriter : public logwriter
{
public:
	sdfatlogwriter( SdFat &sd );
	bool init();

	virtual void log( CATEGORY category, ts &datetime, uint16_t rid, const char* message );
	virtual void log( CATEGORY category, ts &datetime, uint16_t rid, const __FlashStringHelper *message );

	bool dump( Print *p );
	bool truncate();

//protected:
	class sdfwbuffer: public writebuffer
	{
	public:
		sdfwbuffer( FatFile* dir, uint16_t dirindex, void *buf, uint8_t size );
		virtual ~sdfwbuffer();
		bool write( uint16_t data, uint8_t digits );
		bool write( ts &dt );
		virtual bool flush();
	private:
		SdFile	m_f;
	};

protected:

	SdFat		&m_sd;
	uint16_t	m_dirindex;

	const char * 	writecatstr( sdfwbuffer &wb, CATEGORY c );
	char *			writelinehdr( sdfwbuffer &wb, CATEGORY c, ts &datetime, uint16_t remoteid );

};

#endif /* SDFATLOGWRITER_H_ */
