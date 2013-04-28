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
 
#ifndef _netkit_runloop_h
#define _netkit_runloop_h

#include <NetKit/NKObject.h>
#if defined( WIN32 )
#	include <WinSock2.h>
#endif
#include <functional>
#include <ctime>

namespace netkit {

class NETKIT_DLL runloop : public object
{
public:

	typedef void *event;
	
	enum class event_mask
	{
		read		= ( 1 << 2 ),
		write		= ( 1 << 3 ),
		oob			= ( 1 << 4 ),
		timer		= ( 1 << 5 )
	};
	
	typedef smart_ref< runloop > ref;

	typedef std::function< void ( void ) >		dispatch_f;
	typedef std::function< void ( event e ) >	event_f;

	static runloop::ref
	instance();
	
	virtual event
	create( int fd, event_mask m ) = 0;

#if defined( WIN32 )

	virtual event
	create( HANDLE h ) = 0;

#endif
	
	virtual event
	create( std::time_t msec ) = 0;
	
	virtual void
	modify( event e, std::time_t msec ) = 0;
	
	virtual void
	schedule( event e, event_f func ) = 0;
	
	virtual void
	suspend( event e ) = 0;
	
	virtual void
	cancel( event e ) = 0;
	
	virtual void
	dispatch_on_main_thread( dispatch_f f ) = 0;

	virtual void
	run() = 0;
	
	virtual void
	stop() = 0;
};

}

inline netkit::runloop::event_mask
operator|( netkit::runloop::event_mask a, netkit::runloop::event_mask b )
{
	typedef std::underlying_type< netkit::runloop::event_mask >::type enum_type;
	return static_cast< netkit::runloop::event_mask >( static_cast< enum_type >( a ) | static_cast< enum_type >( b ) );
}

inline bool
operator&( netkit::runloop::event_mask a, netkit::runloop::event_mask b )
{
	typedef std::underlying_type< netkit::runloop::event_mask >::type enum_type;
	return ( static_cast< enum_type >( a ) & static_cast< enum_type >( b ) ) ? true : false;
}

#endif
