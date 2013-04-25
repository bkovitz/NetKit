/*
 * Copyright (c) 2013, Porchdog Software Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of the FreeBSD Project.
 *
 */
 
#ifndef _netkit_platform_h
#define _netkit_platform_h

#include <NetKit/NKObject.h>
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

std::string NETKIT_DLL
machine_name();

std::string NETKIT_DLL
machine_description();

std::string NETKIT_DLL
machine_id();

std::string NETKIT_DLL
uuid();

bool NETKIT_DLL
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


inline void
set_error( int error )
{
#if defined( WIN32 )

	SetLastError( error );
	
#else

	errno = error;

#endif
}

}

}

#endif
