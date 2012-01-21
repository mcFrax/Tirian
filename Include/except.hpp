#ifndef _EXCEPT_HPP_
#define _EXCEPT_HPP_

#include <logging.hpp>
#include <stdexcept>
#include <string>
#include <list>

typedef std::runtime_error stdrerror;
typedef std::logic_error stdlerror;

class TreeError
{
	protected:
		std::list<std::string> list;
		TreeError() {}
	public:
		TreeError( const TreeError& pattern ) : list( pattern.list ) {}
		virtual TreeError& operator += ( const std::string& s )
		{
			list.push_back( s );
			return *this;
		}
		virtual void write( FILE* stream ) {};
};

class RuntimeError : public stdrerror, public TreeError
{
	private:
	public:
		RuntimeError( const RuntimeError& pat ) throw()
			: stdrerror( pat.what() ), TreeError( static_cast<TreeError>(pat) ) {}
		RuntimeError( const std::string& s ) throw() : stdrerror( s ) {}
		~RuntimeError() throw() {}
};

class LogicError : public stdlerror, public TreeError
{
	private:
	public:
		LogicError( const LogicError& pat ) throw()
			: stdlerror( pat.what() ), TreeError( static_cast<TreeError>(pat) ) {}
		LogicError( const std::string& s ) throw() : stdlerror( s ) {}
		~LogicError() throw() {}
};

//~ #define RE std::runtime_error
//~ #define LE std::logic_error

#define RE RuntimeError
#define LE LogicError

#define MKRE( x ) RE( x )
#define MKLE( x ) LE( x )
#define MKRERROR2( errtext, ERR ) RE( ( std::string( __PRETTY_FUNCTION__ ) +=  ": \n" errtext ) += ERR )
#define MKRE_LOAD_ERROR( what, from, ERR ) RE( ( ( ( std::string( __PRETTY_FUNCTION__ ) +=  ": \nError while loading " what " from \"" ) += from ) += "\" :\n" ) += ERR )
#define MKLE_LOAD_ERROR( what, from, ERR ) LE( ( ( ( std::string( __PRETTY_FUNCTION__ ) +=  ": \nError while loading " what " from \"" ) += from ) += "\" :\n" ) += ERR )

#ifdef EXCEPTION_TREE
	#define STDRERROR RE( ( std::string( __PRETTY_FUNCTION__ ) += ": \n" ) += ex.what() )
	#define STDLERROR LE( ( std::string( __PRETTY_FUNCTION__ ) += ": \n" ) += ex.what() )
	#define STDRTHROW throw STDRERROR;
	#define STDLTHROW throw STDLERROR;
	#define STDTHROW STDRTHROW
	#define TRY				\
			try				\
			{

	#define CATCH							\
			}								\
			catch( LE &ex )	\
			{								\
				STDLTHROW					\
			}								\
			catch( RE &ex )	\
			{								\
				STDRTHROW					\
			}
	
	#define CATCH_LOAD( WHAT, from )		\
			}								\
			catch( LE &le )					\
			{								\
				throw MKRE_LOAD_ERROR( WHAT, from, le.what() );	\
			}								\
			catch( RE &re )					\
			{								\
				throw MKLE_LOAD_ERROR( WHAT, from, re.what() );	\
			}
#else
	#define STDRERROR
	#define STDLERROR
	#define TRY
	#define CATCH
	#define STDTHROW throw
	#define STDRTHROW throw
	#define STDLTHROW throw
#endif	//EXCEPTION_TREE


#endif //_EXCEPT_HPP_
