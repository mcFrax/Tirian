#ifndef _INTERFACE_HPP_
#define _INTERFACE_HPP_

#include <SDL/SDL.h>

/*!
 *\file interface.hpp
 *\brief Narzędzia do komunikacji z systemem i urządzeniami.
 */

class Widget;

class Mouse
{
	private:
		static long x, y;
		static bool Visible;
		static bool left, middle, right;
	public:
		static void refresh( const long x, const long y ) { Mouse::x = x; Mouse::y = y; }
		static long getX() __attribute__((pure)) { return x; }
		static long getY() __attribute__((pure)) { return y; }
};

#endif //_INTERFACE_HPP_
