/*
 * extdb.h
 *
 *  Created on: Nov 11, 2015
 *      Author: abody
 */

#ifndef THINDB_H_
#define THINDB_H_

#include <sdfile.h>
#include "database.h"


class thindb: public database
{
public:
	bool init(const char *name, bool open);
	bool open();
	bool close();

	bool isinitsucceeded() { return true; }

	virtual bool getParams( int code, dbrecord &recout );
	virtual bool setParams( int code, const dbrecord &recin );
	virtual bool setInfo( int code, const dbrecord& recin );
	virtual bool setStatus( int code, dbrecord::POSITION pos );
	virtual void cleanstatuses();

private:
	bool set(int code, const dbrecord &recin, uint8_t size);

	SdFile		m_file;
	const char *m_name;
};

#endif /* INTDB_H_ */
