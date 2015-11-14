/*
 * dbtest.cpp
 *
 *  Created on: Nov 14, 2015
 *      Author: compi
 */
#include <Serial.h>
#include <string>
#include "../../gatelogic/extdb.cpp"

using namespace std;

char	g_serialbuffer[256];

bool test_db_getParams( extdb &db, dbrecord &dbr )
{
	Serial.reset( ":1C2 438 000 59F 07F 001\n" );

	bool		ret(db.getParams( 0, dbr ));
	bool 		succ( ret && dbr.in_start == 0x1c2 && dbr.in_end == 0x438 && dbr.out_start == 0 &&
						dbr.out_end == 0x59f && dbr.days == 0x7f && (int)dbr.position == 1 );

	return succ && Serial.getoutput().str() == "get 0\n";
}

bool test_db_setparams( extdb &db, const dbrecord &dbr )
{
	Serial.reset(":OK\n");
	bool result( db.setParams(0, dbr ));
	auto str( Serial.getoutput().str());
	return result && str == "set 0 1C2 438 000 59F 07F 001\n";
}

bool test_db_setstatus( extdb &db )
{
	Serial.reset( ":OK\n" );
	bool result( db.setStatus( 0, dbrecord::inside ));
	auto str( Serial.getoutput().str());
	return result && str == "set 0 002\n";
}

int main()
{
	extdb		db( g_serialbuffer, sizeof( g_serialbuffer ));
	dbrecord	dbr;
	bool succ( true );
	succ &= test_db_getParams( db, dbr );
	succ &= test_db_setparams( db, dbr );
	succ &= test_db_setstatus( db );
	return succ ? 0 : -1;
}
