/*
 * serialout.h
 *
 *  Created on: Feb 6, 2016
 *      Author: compi
 */

#ifndef COMMON_SERIALOUT_H_
#define COMMON_SERIALOUT_H_

template<typename Sep, typename Arg1> void serialoutsepln( const Sep sep, const Arg1 arg1 ) {
	Serial.print( arg1 );
	Serial.println();
}
template< typename Sep, typename Arg1, typename... Args> void serialoutsepln(const Sep sep, const Arg1& arg1, const Args&... args) {
	Serial.print( arg1 );
	Serial.print( sep );
	serialoutsepln( sep, args...);
}

template<typename Sep, typename Arg1> void serialoutsep( const Sep sep, const Arg1 arg1 ) {
	Serial.print( arg1 );
	Serial.print( sep );
}
template< typename Sep, typename Arg1, typename... Args> void serialoutsep(const Sep sep, const Arg1& arg1, const Args&... args) {
	Serial.print( arg1 );
	Serial.print( sep );
	serialoutsep( sep, args...);
}

template< typename Arg1 > void serialoutln( const Arg1& arg1 )
{
	Serial.println( arg1 );
}

template< typename Arg1, typename... Args> void serialoutln( const Arg1& arg1, const Args&... args)
{
	Serial.print( arg1 );
	serialoutln( args...);
}

template< typename Arg1 > void serialout( const Arg1& arg1 )
{
	Serial.print( arg1 );
}

template< typename Arg1, typename... Args> void serialout( const Arg1& arg1, const Args&... args)
{
	Serial.print( arg1 );
	serialout( args...);
}



#endif /* COMMON_SERIALOUT_H_ */
