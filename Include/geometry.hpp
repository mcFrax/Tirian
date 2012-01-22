#ifndef _GEOMETRY_HPP_
#define _GEOMETRY_HPP_

#include <cmath>

class Point2f
{
	private:
		float tab[ 2 ];
	public:
		Point2f() {}
		Point2f( float a, float b ) { tab[ 0 ] = a; tab[ 1 ] = b; }
		void set( float a, float b ) { tab[ 0 ] = a; tab[ 1 ] = b; }
		float & x() { return tab[ 0 ]; }
		float & y() { return tab[ 1 ]; }
		const float & x() const { return tab[ 0 ]; }
		const float & y() const { return tab[ 1 ]; }
		operator float*() { return tab; }
};

class Point3f
{
	private:
		float tab[ 3 ];
	public:
		Point3f() {}
		Point3f( float a, float b, float c ) { tab[ 0 ] = a; tab[ 1 ] = b; tab[ 2 ] = c; }
		void set( float a, float b, float c ) { tab[ 0 ] = a; tab[ 1 ] = b; tab[ 2 ] = c; }
		float & x() { return tab[ 0 ]; }
		float & y() { return tab[ 1 ]; }
		float & z() { return tab[ 2 ]; }
		const float & x() const { return tab[ 0 ]; }
		const float & y() const { return tab[ 1 ]; }
		const float & z() const { return tab[ 2 ]; }
		operator float*() { return tab; }
};

//!Dwuwymiarowa funkcja liniowa
class Line2
{
	// f(x) = ax + b
	public:
		float a, b;
		void set( const float _a, const float _b );
		void set( const Point2f p1, const Point2f p2 )
		{
			if ( p1.x() == p2.x() ){
				a = NAN; b = p1.x();
			} else {
				a = ( p1.y()-p2.y() ) / ( p1.x()-p2.x() );
				b = p1.y() - p1.x()*a;
			}
		}
		Line2( const Point2f p1, const Point2f p2 ) { set( p1, p2 ); }
		Line2( const float _a, const float _b ) : a(_a), b(_b) {}
		Line2() : a(0), b(0) {}
		float x( const float y ) const
		{
			if ( isnanf( a ) )
				return b;
			else
				if ( a == 0 )
					if ( y == b )
						return INFINITY;
					else
						return NAN;
				else
					return (y-b) / a;
		}
		float operator()( const float x ) const
		{
			if ( isnanf( a ) )
				if ( x == b )
					return INFINITY;
				else
					return NAN;
			else
				return a*x+b;
		}
		Line2& operator =( const Line2 p ) { a = p.a; b = p.b; return *this; }
		Line2& operator+=( const Line2 p ) { a += p.a; b += p.b; return *this; }
		Line2& operator-=( const Line2 p ) { a -= p.a; b -= p.b; return *this; }
};

inline Line2 operator-( const Line2 a, const Line2 b ) { return Line2( a.a - b.a, a.b - b.b ); }
inline Line2 operator+( const Line2 a, const Line2 b ) { return Line2( a.a + b.a, a.b + b.b ); }

//!Trójwymiarowa funkcja liniowa
class Line3
{
	private:
		Line2 X;
		Line2 Y;
	public:
		Line3();
		Line3( const Point3f p1, const Point3f p2 )
			: X ( Point2f( p1.z(), p2.x() ), Point2f( p1.z(), p2.x() ) ), Y ( Point2f( p1.z(), p1.y() ), Point2f( p2.z(), p2.y() ) ) {}
		Point2f operator()( const float z ) { return Point2f( X(z), Y(z) ); }
};

#endif //_GEOMETRY_HPP_
