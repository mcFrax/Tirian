#ifndef _INTERFACE_HPP_
#define _INTERFACE_HPP_

#include <SDL/SDL.h>

/*!
 *\file interface.hpp
 *\brief Narzędzia do komunikacji z systemem i urządzeniami.
 */

class Engine;

class Mouse
{
	friend class Engine;
	private:
		static long x, y;
		static bool Visible;
		static bool buttonState[6];
		static void refresh( const long x, const long y ) { Mouse::x = x; Mouse::y = y; }
		static void setButton( long button, bool state ) { buttonState[ button ] = state; }
	public:
		static long getX()  { return x; }
		static long getY()  { return y; }
		static bool button( const long i ) { return buttonState[i]; };
};

#endif //_INTERFACE_HPP_
