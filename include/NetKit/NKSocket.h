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
 
#ifndef _netkit_socket_h
#define _netkit_socket_h

#include <NetKit/NKRunLoop.h>
#include <NetKit/NKEndpoint.h>
#include <NetKit/NKSource.h>
#include <NetKit/NKSink.h>
#include <initializer_list>
#include <functional>
#include <errno.h>
#if defined( WIN32 )

#else
#	include <sys/socket.h>
#	include <unistd.h>
#	include <fcntl.h>
#endif

namespace netkit {

class socket : public source
{
public:

	typedef smart_ptr< socket > ptr;
	
#if defined( WIN32 )

	typedef SOCKET native;
	static const native null = INVALID_SOCKET;

#else

	typedef int native;
	static const native null = -1;

#endif
	
	enum class error
	{
#if defined( WIN32 )
		reset		=	WSAECONNRESET,
		aborted		=	WSAECONNABORTED,
		in_progress	=	WSAEINPROGRESS,
		would_block	=	WSAEWOULDBLOCK
#else
		reset		=	ECONNRESET,
		aborted		=	ECONNABORTED,
		in_progress	=	EINPROGRESS,
		would_block	=	EAGAIN
#endif
	};
	
	class adapter : public source::adapter
	{
	public:
	
		typedef smart_ptr< adapter > ptr;
		
		virtual void
		connect( const endpoint::ptr &endpoint, connect_reply_f reply );
		
		virtual void
		accept( accept_reply_f reply );
		
		virtual void
		peek( peek_reply_f reply );
	
		virtual void
		recv( recv_reply_f reply );
	
		virtual std::streamsize
		send( const std::uint8_t *buf, std::size_t len );
		
	private:
	
		std::uint8_t m_buf[ 4192 ];
	};
	
	static bool
	set_blocking( native fd, bool block );
	
	inline bool
	set_blocking( bool val )
	{
		return set_blocking( m_fd, val );
	}

	virtual bool
	is_open() const;
	
	virtual void
	close();
	
	inline bool
	connected() const
	{
		return m_connected;
	}
	
	void
	on_event( runloop::event_mask mask, runloop::event_f func );
	
	inline native
	fd() const
	{
		return m_fd;
	}
	
protected:

	socket( int domain, int type );

	socket( native fd );
	
	socket( native fd, const endpoint::ptr peer );
	
	socket( const socket &that );	// Not implemented
	
	virtual ~socket();
	
	void
	init();
	
	bool			m_connected;
	endpoint::ptr	m_peer;
	runloop::event	m_event;
	native			m_fd;
};


class acceptor : public object
{
public:

	typedef std::function< void ( int status, socket::ptr sock ) >	accept_reply_f;
	typedef smart_ptr< acceptor >									ptr;
	
	acceptor( const endpoint::ptr &endpoint, int domain, int type );
	
	acceptor( socket::native fd );
	
	virtual ~acceptor();
	
	int
	listen( int size );
	
	virtual void
	accept( accept_reply_f reply ) = 0;
	
	inline const endpoint::ptr&
	endpoint() const
	{
		return m_endpoint;
	}
	
protected:

	endpoint::ptr		m_endpoint;
	runloop::event		m_event;
	socket::native		m_fd;
	
private:

	acceptor( const acceptor &that );	// Not implemented
};

namespace ip {

class socket : public netkit::socket
{
public:

	void
	connect( const uri::ptr &uri, connect_reply_f reply );

	const ip::endpoint::ptr&
	peer();
	
protected:

	socket( int domain, int type );

	socket( native fd );
	
	socket( native fd, const ip::endpoint::ptr &peer );
	
	socket( const socket &that );	// Not implemented
};

class acceptor : public netkit::acceptor
{
public:

	acceptor( const ip::endpoint::ptr &endpoint, int type );
	
	acceptor( socket::native fd );
	
	inline ip::endpoint::ptr
	endpoint() const
	{
		return dynamic_pointer_cast< ip::endpoint, netkit::endpoint >( m_endpoint );
	}
	
private:

	acceptor( const acceptor &that );	// Not implemented
};

namespace tcp {

class socket : public ip::socket
{
public:

	typedef smart_ptr< socket > ptr;
	
	socket();
	
	socket( native fd );
	
	socket( native fd, const ip::endpoint::ptr &peer );
	
	virtual ~socket();
	
protected:

	std::string m_peer_ethernet_addr;
};

class acceptor : public ip::acceptor
{
public:

	typedef smart_ptr< acceptor > ptr;

	acceptor( const ip::endpoint::ptr &endpoint );

	virtual ~acceptor();
	
	virtual void
	accept( accept_reply_f reply );
};

}

}

}

#endif