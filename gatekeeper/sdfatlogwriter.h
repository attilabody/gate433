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


	void log( 	CATEGORY category, ts &datetime, const char* message,
				uint16_t rid = 0xffff, uint8_t dbpos = 0xff,
				uint8_t loop = 0xff, uint8_t decision = 0xff );

	void log( 	CATEGORY category, ts &datetime,
				const __FlashStringHelper *message,
				uint16_t rid = 0xffff, uint8_t dbpos = 0xff,
				uint8_t loop = 0xff, uint8_t decision = 0xff );

	bool dump( Print *p );
	bool truncate();

protected:
	class sdfwbuffer: public writebuffer
	{
	public:
		sdfwbuffer( FatFile* dir, uint16_t dirindex, void *buf, uint8_t size );
		~sdfwbuffer();
		bool write( uint16_t data, uint8_t digits );
		bool write( ts &dt );
		virtual bool flush();
	private:
		SdFile	m_f;
	};

protected:

	SdFat		&m_sd;
	uint16_t	m_dirindex;
	bool		m_initialized;

	bool	writelinehdr( sdfwbuffer &wb, CATEGORY c, ts &datetime, uint16_t remoteid, uint8_t dbpos, uint8_t loop, uint8_t decision );

};

#endif /* SDFATLOGWRITER_H_ */
