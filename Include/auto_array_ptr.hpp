#ifndef _AUTO_ARRAY_PTR_HPP_
#define _AUTO_ARRAY_PTR_HPP_

/*! 
 * \file auto_array_ptr.hpp
 * \brief Implementacja klasy wzorcowej auto_array_ptr
 */

//!Odpowiednik auto_ptr dla tablic
template < class T >
class auto_array_ptr
{
	private:
		T *ptr;
	public:
		auto_array_ptr() { ptr = 0; }
		auto_array_ptr( T *p ) : ptr( p ) {}
		~auto_array_ptr() { if ( ptr != 0 ) delete [] ptr; }
		operator T* () { return ptr; };
		const T* & operator*() { return ptr; };
		auto_array_ptr<T>& operator=( auto_array_ptr<T> &pattern )
		{
			if ( ptr != 0 ) delete [] ptr;
			ptr = pattern.ptr;
			pattern.ptr = 0;
			return *this;
		}
		auto_array_ptr<T>& operator=( T *p ) { reset(); ptr = p ; return *this; }
		void reset(){
			if ( ptr != 0 ) delete [] ptr;
			ptr = 0;
		}
		void release(){
			ptr = 0;
		}
};

#endif // _AUTO_ARRAY_PTR_HPP_
