/*
 * commands.h
 *
 *  Created on: Feb 5, 2018
 *      Author: compi
 */

#ifndef COMMANDS_H_
#define COMMANDS_H_

const char CMD_GETDBRECORD[] = "gdb";	//	get db record
const char CMD_SETDBRECORD[] = "sdb";	//	set db record
const char CMD_EXPORTDB[] = "edb";	//	export database
const char CMD_IMPORTDB[] = "idb";	//	import database
const char CMD_DUMPDB[] = "ddb";	//	dump database
const char CMD_GETDATETIME[] = "gdt";	//	get datetime
const char CMD_SETDATETIME[] = "sdt";	//	set datetime
const char CMD_CLEARSTATUS[] = "cs";		//	clear statuses
const char CMD_DUMPSHUFFLE[] = "ds";		//	dump shuffle
const char CMD_DUMPLOG[] = "dl";		//	dump log
const char CMD_TRUNCATELOG[] = "tl";		//	truncate log
const char CMD_HALT[] = "halt";		//	infinite loop
const char CMD_GETTEMP[] = "gt";		//	get temperature
const char CMD_GETCONFIG[] = "get";	//  get configuration variable
const char CMD_SETCONFIG[] = "set";	//  set configuration variable
const char CMD_LISTCONFIG[] = "list";
const char CMD_SAVECONFIG[] = "save";	//	save configuration
const char CMD_LOADCONFIG[] = "load";
const char CMD_CLEARCONFIG[] = "clr";
const char CMD_SETLIGHTS[] = "sl";		//	set lights

char const CFG_LCDI2CADDRESS[] = "lia";
char const CFG_PASSTIMEOUT[] = "pto";
char const CFG_HURRYTIMEOUT[] = "hto";
char const CFG_RELAXEDPOS[] = "rpo";
char const CFG_RELAXEDDATETIME[] = "rdt";

#endif /* COMMANDS_H_ */
