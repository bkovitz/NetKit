#ifndef _coreapp_cstring_h
#define _coreapp_cstring_h

#include <stdarg.h>

namespace std {

inline void*
memmove_s( void *s1, size_t size, const void *s2, size_t n )
{
#if defined( WIN32 )

	return ( void* ) ::memmove_s( s1, size, s2, n );

#else

	return ::memmove( s1, s2, n );

#endif
}

inline int
strncasecmp( const char *buf1, const char *buf2, size_t len )
{
#if defined( WIN32 )

	return _strnicmp( buf1, buf2, len );

#else

	return ::strncasecmp( buf1, buf2, len );
#endif
}

inline int
sprintf_s( char *buffer, size_t buflen, size_t count, const char *format, ... )
{
	va_list argList;
	
	va_start( argList, format );
	
#if defined( WIN32 )

	int ret = _vsnprintf_s( buffer, buflen, count, format, argList );

#else

	int ret = vsprintf( buffer, format, argList );
	
#endif

	va_end( argList );
	
	return ret;
}

}

#endif
