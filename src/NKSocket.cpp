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
 
#include <NetKit/NKSocket.h>
#include <NetKit/NKPlatform.h>
#include <NetKit/NKLog.h>
#if defined( WIN32 )
#include <WinSock2.h>
#include <WS2tcpip.h>
#else
#	include <sys/socket.h>
#	include <arpa/inet.h>
#	include <unistd.h>
#	include <fcntl.h>
#endif
#include <thread>

using namespace netkit;

#if defined( __APPLE__ )
#	pragma mark socket implementation
#endif



socket::socket()
{
}


socket::socket( int domain, int type )
{
	m_fd = runloop::main()->create( domain, type, 0 );
}


socket::socket( runloop::fd::ref fd )
:
	m_fd( fd )
{
}


socket::socket( runloop::fd::ref fd, const endpoint::ref peer )
:
	m_peer( peer ),
	m_fd( fd )
{
}


socket::~socket()
{
	nklog( log::verbose, "" );

	close();
}





#if 0
int
socket::finish_connect()
{
	// We'll use a trick to determine if the connect worked as described here:
	//
	// http://cr.yp.to/docs/connect.html
	//
	
	sockaddr_storage	addr;
	socklen_t			len = sizeof( addr );
					
	return getpeername( m_fd, ( struct sockaddr* ) &addr, &len );
}
#endif


void
socket::start_connect( const endpoint::ref &peer, source::connect_reply_f reply )
{
	sockaddr_storage addr;
	
	peer->to_sockaddr( addr );
	
	m_fd = runloop::main()->create( addr.ss_family, SOCK_STREAM, 0 );

	if ( m_fd )
	{
		m_fd->connect( peer, reply );
	}
	else
	{
		nklog( log::error, "unable to create socket" );
		reply( -1, nullptr );
	}
}


void
socket::start_send( const std::uint8_t *buf, std::size_t len, source::send_reply_f reply )
{
	m_fd->send( buf, len, [=]( int status )
	{
		if ( status )
		{
			nklog( log::error, "send returned %", platform::error() );
		}

		reply( status );
	} );
}

	
void
socket::start_recv( source::recv_reply_f reply )
{
	m_fd->recv( [=]( int status, const std::uint8_t *buf, std::size_t len  )
	{
		if ( status )
		{
			nklog( log::error, "recv returned %", platform::error() );
		}

		reply( status, buf, len );
	} );
}


void
socket::close( bool notify )
{
	teardown_notifications();

	if ( m_fd )
	{
		m_fd->close();
		m_fd = nullptr;
	}

	source::close( notify );
}


bool
socket::is_open() const
{
	return ( m_fd ) ? true : false;
}


endpoint::ref
socket::peer() const
{
	return m_peer;
}


#if defined( __APPLE__ )
#	pragma mark acceptor implementation
#endif

acceptor::acceptor( const endpoint::ref &endpoint, int domain, int type )
{
	m_fd = runloop::main()->create( endpoint, m_endpoint, domain, type, 0 );
}


acceptor::~acceptor()
{
	nklog( log::verbose, "" );

	if ( m_fd )
	{
		m_fd->close();
		m_fd = nullptr;
	}
}


void
acceptor::close()
{
	if ( m_fd )
	{
		m_fd->close();
		m_fd = nullptr;
	}
}


#if defined( __APPLE__ )
#	pragma mark ip::socket implementation
#endif

ip::socket::socket( int domain, int type )
:
	netkit::socket( domain, type )
{
}


ip::socket::socket( runloop::fd::ref fd, const ip::endpoint::ref &peer )
:
	netkit::socket( fd )
{
	m_peer = peer;
}

#if defined( __APPLE__ )
#	pragma mark ip::acceptor implementation
#endif

ip::acceptor::acceptor( const ip::endpoint::ref &endpoint, int type )
:
	netkit::acceptor( endpoint, endpoint->addr()->is_v4() ? AF_INET : AF_INET6, type )
{
}


#if defined( __APPLE__ )
#	pragma mark ip::tcp::socket implementation
#endif

ip::tcp::socket::socket()
:
	ip::socket( AF_INET, SOCK_STREAM )
{
	set_keep_alive( true );
}


ip::tcp::socket::socket( runloop::fd::ref fd, const ip::endpoint::ref &peer )
:
	ip::socket( fd, peer )
{
}


ip::tcp::socket::~socket()
{
}


void
ip::tcp::socket::set_keep_alive( bool val )
{
	int toggle = ( val ) ? 1 : 0;

	setsockopt( m_fd, SOL_SOCKET, SO_KEEPALIVE, ( char* ) &toggle, sizeof( toggle ) );
}


#if defined( __APPLE__ )
#	pragma mark ip::tcp::acceptor implementation
#endif

ip::tcp::acceptor::acceptor( const ip::endpoint::ref &endpoint )
:
	ip::acceptor( endpoint, SOCK_STREAM )
{
}


ip::tcp::acceptor::~acceptor()
{
	nklog( log::verbose, "" );
}


void
ip::tcp::acceptor::accept( std::size_t peek, accept_reply_f reply )
{
	assert( m_fd );

	if ( m_fd )
	{
		m_fd->accept( peek, [=]( int status, runloop::fd::ref fd, const netkit::endpoint::ref &peer, const std::uint8_t *peek_buf, std::size_t peek_len )
		{
			if ( status == 0 )
			{
				socket::ref new_sock;
			
				new_sock = new ip::tcp::socket( fd, ( ip::endpoint* ) peer.get() );
				
				reply( 0, new_sock.get(), peek_buf, peek_len );
			}
			else
			{
			}
		} );
	}
}
