#ifndef _LOG_HPP_
#define _LOG_HPP_

#include <string>
#include <iostream>
#include <cstdarg>
#include <cstdio>

#define QUOT( x ) #x

#define FOOTPRINT printf( __FILE__":%i: in %s : footprint\n", __LINE__, __PRETTY_FUNCTION__ );
#define FP FOOTPRINT
	

#if 0
	#define LOG( x ) \
		do {	\
		printf( "%s: ", __PRETTY_FUNCTION__ );	\
		printf x;	\
		} while(0)
#else
	#define LOG( x ) \
		printf x;
#endif

#define CLOG	\
	std::clog << __PRETTY_FUNCTION__": "


#ifdef debug
	#define LOGdeb( x )	LOG( x )
#else
	#define LOGdeb( x )
#endif

#define PRINT( format, x ) printf( #x " = %"QUOT(format)"\n", x )
#define PRINTI( x ) printf( #x " = %i\n", x )
#define PRINTS( x ) printf( #x " = %s\n", x )
#define PRINTF( x ) printf( #x " = %f\n", x )


#endif
