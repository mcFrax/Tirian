#include <hash.hpp>
#include <cctype>

static const unsigned long long p = 987654323;
static const unsigned long long mod = 1000000097;

unsigned hash( const char* s )
{
	int h = 0;
	for ( unsigned i = 0; s[i] != '\0'; ++i )
		((h*=p)+=s[i])%=mod;
	return h;
}

unsigned NCShash( const char* s ) //non case-sensitive hash
{
	int h = 0;
	for ( unsigned i = 0; s[i] != '\0'; ++i ){
		((h*=p)+=tolower(s[i]))%=mod;
	}
	return h;
}
