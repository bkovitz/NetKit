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
#include <NetKit/NKEndpoint.h>
#if defined( _WIN32 )
#	include <WinSock2.h>
#endif
#include <functional>
#include <ctime>

namespace netkit {

class NETKIT_DLL runloop : public object
{
public:

	class NETKIT_DLL fd : public object
	{
	public:

		typedef smart_ref< fd >																								ref;
		typedef std::function< void ( int status, const endpoint::ref &peer ) >												connect_reply_f;
		typedef std::function< void ( int status, fd::ref fd, const endpoint::ref &peer ) >									accept_reply_f;
		typedef std::function< void ( int status ) >																		send_reply_f;
		typedef std::function< void ( int status, const std::uint8_t *buf, std::size_t len ) >								recv_reply_f;
		typedef std::function< void ( int status, const std::uint8_t *buf, std::size_t len, netkit::endpoint::ref from ) >	recvfrom_reply_f;

		virtual int
		bind( netkit::endpoint::ref to ) = 0;

		virtual void
		connect( netkit::endpoint::ref to, connect_reply_f reply ) = 0;

		virtual void
		accept( accept_reply_f reply ) = 0;

		virtual void
		send( const std::uint8_t *buf, std::size_t len, send_reply_f reply ) = 0;

		virtual void
		sendto( const std::uint8_t *buf, std::size_t len, netkit::endpoint::ref to, send_reply_f reply ) = 0;

		virtual void
		recv( recv_reply_f reply ) = 0;

		virtual void
		recvfrom( recvfrom_reply_f reply ) = 0;

		virtual void
		close() = 0;
	};

	typedef void *event;
	
	enum class event_mask
	{
		timer = ( 1 << 0 )
	};

	enum class mode
	{
		normal			= 0,
		once			= 1,
#if defined( _WIN32 )
		input_events	= 2,
#endif
	};

	typedef smart_ref< runloop >				ref;
	typedef std::function< void ( void ) >		dispatch_f;
	typedef std::function< void ( event e ) >	event_f;

	static runloop::ref
	main();

	virtual fd::ref
	create( std::int32_t domain, std::int32_t type, std::int32_t protocol ) = 0;

	virtual fd::ref
	create( netkit::endpoint::ref in_endpoint, netkit::endpoint::ref &out_endpoint, std::int32_t domain, std::int32_t type, std::int32_t protocol ) = 0;

#if defined( _WIN32 )
	
	virtual event
	create( HANDLE h ) = 0;

#endif

	virtual event
	create( std::time_t msec ) = 0;
	
	virtual void
	schedule( event e, event_f func ) = 0;

	virtual void
	schedule_oneshot_timer( std::time_t msec, event_f func ) = 0;
	
	virtual void
	suspend( event e ) = 0;
	
	virtual void
	cancel( event e ) = 0;
	
	virtual void
	dispatch( dispatch_f f ) = 0;

	virtual void
	run( mode how = mode::normal ) = 0;
	
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
