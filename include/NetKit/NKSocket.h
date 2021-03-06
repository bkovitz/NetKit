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
#include <functional>
#include <errno.h>
#if defined( WIN32 )

#else
#	include <sys/socket.h>
#	include <unistd.h>
#	include <fcntl.h>
#endif

namespace netkit {

class NETKIT_DLL socket : public source
{
public:

	typedef smart_ref< socket > ref;
	
#if defined( WIN32 )

	typedef const char * const_buf_t;
	typedef char* buf_t;

#else

	typedef const void* const_buf_t;
	typedef void* buf_t;

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
	
	virtual bool
	is_open() const;
	
	virtual void
	close( bool notify = true );
	
	inline bool
	connected() const
	{
		return m_connected;
	}
	
	void
	on_event( runloop::event_mask mask, runloop::event_f func );
	
	virtual endpoint::ref
	peer() const;
	
	inline netkit::runloop::fd::ref
	fd() const
	{
		return m_fd;
	}

	socket();

protected:

	class adapter : public source::adapter
	{
	public:
	
		typedef adapter						*ref;
		typedef intrusive_list< adapter >	list;
		
		adapter();
		
		virtual ~adapter();
		
		virtual void
		resolve( const uri::ref &uri, resolve_reply_f reply );
	
		virtual void
		connect( const uri::ref &uri, const endpoint::ref &to, connect_reply_f reply );
		
		virtual void
		send( const std::uint8_t *in_buf, std::size_t in_len, send_reply_f reply );
		
		virtual void
		recv( const std::uint8_t *in_buf, std::size_t in_len, recv_reply_f reply );
	
	protected:
	
		friend class source;
		
		source::ref m_source;
	};

	socket( int domain, int type );

	socket( netkit::runloop::fd::ref fd );
	
	socket( netkit::runloop::fd::ref fd, const endpoint::ref peer );
	
	socket( const socket &that );	// Not implemented
	
	virtual ~socket();
	
	virtual void
	start_connect( const endpoint::ref &peer, source::connect_reply_f reply );
	
	virtual void
	start_send( const std::uint8_t *buf, std::size_t len, source::send_reply_f reply );
	
	virtual void
	start_recv( source::recv_reply_f reply ); 

	bool				m_connected;
	endpoint::ref		m_peer;
	runloop::fd::ref	m_fd;
};


class NETKIT_DLL acceptor : public object
{
public:

	typedef std::function< void ( int status, socket::ref sock, const std::uint8_t *peek_buf, std::size_t peek_len ) >	accept_reply_f;
	typedef smart_ref< acceptor >																						ref;
	
	acceptor( const endpoint::ref &endpoint, int domain, int type );
	
	virtual ~acceptor();

	inline bool
	is_listening() const
	{
		return ( m_fd ) ? true : false;
	}
	
	virtual void
	accept( std::size_t peek, accept_reply_f reply ) = 0;
	
	inline const netkit::endpoint::ref&
	endpoint() const
	{
		return m_endpoint;
	}

	virtual void
	close();
	
protected:

	netkit::endpoint::ref   m_endpoint;
	runloop::fd::ref		m_fd;
	
private:

	acceptor( const acceptor &that );	// Not implemented
};

namespace ip {

class NETKIT_DLL socket : public netkit::socket
{
public:

	const ip::endpoint::ref&
	peer();
	
protected:

	socket( int domain, int type );

	socket( runloop::fd::ref fd, const ip::endpoint::ref &peer );
	
	socket( const socket &that );	// Not implemented
};

class NETKIT_DLL acceptor : public netkit::acceptor
{
public:

	typedef smart_ref< acceptor > ref;

	acceptor( const ip::endpoint::ref &endpoint, int type );
	
	inline ip::endpoint::ref
	endpoint() const
	{
		return dynamic_pointer_cast< ip::endpoint, netkit::endpoint >( m_endpoint );
	}
	
private:

	acceptor( const acceptor &that );	// Not implemented
};

namespace tcp {

class NETKIT_DLL socket : public ip::socket
{
public:

	typedef smart_ref< socket > ref;
	
	socket();
	
	socket( runloop::fd::ref fd, const ip::endpoint::ref &peer );
	
	virtual ~socket();

	void
	set_keep_alive( bool val );
	
protected:

	std::string m_peer_ethernet_addr;
};

class NETKIT_DLL acceptor : public ip::acceptor
{
public:

	typedef smart_ref< acceptor > ref;

	acceptor( const ip::endpoint::ref &endpoint );

	virtual ~acceptor();
	
	virtual void
	accept( std::size_t peek, accept_reply_f reply );
};

}

}

}

#endif
