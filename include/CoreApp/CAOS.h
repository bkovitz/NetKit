#ifndef _CoreApp_os_h
#define _CoreApp_os_h

#include <CoreApp/types.h>
#include <CoreApp/tstring.h>

#if defined( WIN32 )

#	include <winsock2.h>
#	include <windows.h>

#else

#	include <sys/socket.h>
#	include <sys/types.h>
#	include <sys/errno.h>
#	include <stdlib.h>
#	include <unistd.h>

#endif


namespace CoreApp {
namespace os {

std::tstring
computer_name();

std::tstring
machine_id();

std::tstring
machine_description();

std::tstring
uuid();

bool
create_folder( const std::tstring& folder );

inline int
catnap( int msec )
{
#if defined( WIN32 )

	Sleep( msec );
	return 0;
	
#else

	return usleep( msec * 1000 );
	
#endif
}


inline int
error()
{
#if defined( WIN32 )

	return GetLastError();
	
#else

	return errno;

#endif
}

}
}

#endif
