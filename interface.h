// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _interface_H_
#define _interface_H_

#define BAUDRATE 38400
#define RESP ":"
#define ERR "!"
#define CMNT "#"

/*
000 59F 000 59F 000007F
*/
#define IN_START_OFFSET		0
#define IN_START_WIDTH		3
#define IN_END_OFFSET		4
#define IN_END_WIDTH		3
#define OUT_START_OFFSET	8
#define OUT_START_WIDTH		3
#define OUT_END_OFFSET		12
#define OUT_END_WIDTH		3
#define FLAGS_OFFSET		16
#define FLAGS_WIDTH			7
#define RECORD_WIDTH		24	// \n added

#endif /* _interface_H_ */
