#ifndef _netkit_dispatch_h
#define _netkit_dispatch_h

#if defined( __APPLE__ )

#	include <dispatch/dispatch.h>

#elif defined( WIN32 )

#	include <Win32/dispatch/dispatch.h>

#endif

#endif
