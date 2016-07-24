// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section
#ifndef _toolbox_H_
#define _toolbox_H_
//#define DBGSERIALIN	1
#include <avr/wdt.h>
#include <Arduino.h>
#define ITEMCOUNT(A) (sizeof(A)/sizeof(A[0]))

inline char halfbytetohex( uint8_t data ) { return data + ( data < 10 ? '0' : ( 'A' - 10 ) ); }
long getintparam( const char* &input, bool decimal = true, bool trimstart = true, bool acceptneg = false );
bool iscommand( const char *&inptr, const char *cmd, bool pgmspace = true );
bool iscommand( const char *&inptr, const __FlashStringHelper *cmd );
char findcommand( const char* &inptr, const char **commands );
bool getlinefromserial( char* buffer, uint8_t buflen, uint8_t &idx );
void hex2serial( uint16_t out, uint8_t digits, const char* prefix );
uint8_t uitohex( char* buffer, uint16_t data, uint8_t digits );
uint8_t ultohex( char* buffer, uint32_t data, uint8_t digits );
uint8_t uitodec( char* buffer, uint16_t data, uint8_t digits );
uint8_t ultodec( char* buffer, uint32_t data, uint8_t digits );

#define CHECKPOINT wdt_reset()


#endif /* _toolbox_H_ */
