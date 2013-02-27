#ifndef _netkit_platform_h
#define _netkit_platform_h

#include <string>

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


namespace netkit {

namespace platform {

std::string
computer_name();

std::string
machine_id();

std::string
machine_description();

std::string
uuid();

bool
create_folder( const std::string& folder );

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
