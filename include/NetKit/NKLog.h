#ifndef _netkit_log_h
#define _netkit_log_h


#if defined( WIN32 )

#	include <winsock2.h>
#	include <windows.h>
#	include <stdarg.h>
#	include <stdio.h>
#	define nklog( LEVEL, MESSAGE, ... ) netkit::log::put( LEVEL, __FILE__, __FUNCTION__, __LINE__, MESSAGE, __VA_ARGS__ );

#else

#	define nklog( LEVEL, MESSAGE, ... ) netkit::log::put( LEVEL, __FILE__, __FUNCTION__, __LINE__, MESSAGE, ##__VA_ARGS__ );

#endif


namespace netkit {

class log
{
public:

	enum level
	{
		error	= 1,
		warning	= 2,
		info	= 3,
		verbose	= 10
	};
		
	static void
#if defined( WIN32 )
	init( LPCTSTR name );
#else
	init( const char *name );
#endif

	static level
	get_level();

	static void
	set_level( level l );

	static void
	put( level l, const char * filename, const char * function, int line, const char * message, ... );
};

}

#endif