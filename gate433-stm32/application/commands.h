/*
 * commands.h
 *
 *  Created on: Feb 5, 2018
 *      Author: compi
 */

#ifndef COMMANDS_H_
#define COMMANDS_H_

const char CMD_GDB[] = "gdb";	//	get db record
const char CMD_SDB[] = "sdb";	//	set db record
const char CMD_EDB[] = "edb";	//	export database
const char CMD_IDB[] = "idb";	//	import database
const char CMD_DDB[] = "ddb";	//	dump database
const char CMD_GDT[] = "gdt";	//	get datetime
const char CMD_SDT[] = "sdt";	//	set datetime
const char CMD_CS[] = "cs";		//	clear statuses
const char CMD_DS[] = "ds";		//	dump shuffle
const char CMD_DL[] = "dl";		//	dump log
const char CMD_TL[] = "tl";		//	truncate log
const char CMD_IL[] = "il";		//	infinite loop
const char CMD_GT[] = "gt";		//	get temperature
const char CMD_SET[] = "set";	//  set configuration variable
const char CMD_SL[] = "sl";		//	set lights

#define CFG_LCDI2CADDRESS "lia"
#define CFG_PASSTIMEOUT "pto"
#define CFG_HURRYTIMEOUT "hto"
#define CFG_RELAXEDPOS "rpo"
#define CFG_RELAXEDDATETIME "rdt"

#endif /* COMMANDS_H_ */
