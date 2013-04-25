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

socket::socket( int domain, int type )
{
	m_fd = ::socket( domain, type, 0 );
	
	if ( m_fd != socket::null )
	{
		init();
	}
}


socket::socket( native fd )
:
	m_fd( fd )
{
	init();
}


socket::socket( native fd, const endpoint::ptr peer )
:
	m_peer( peer ),
	m_fd( fd )
{
	init();
}


socket::~socket()
{
	close();
}


void
socket::init()
{
	set_blocking( m_fd, false );
	
	m_send_event	= runloop::instance()->create( m_fd, runloop::event_mask::write );
	m_recv_event	= runloop::instance()->create( m_fd, runloop::event_mask::read );
}


int
socket::start_connect( const endpoint::ptr &peer, bool &would_block )
{
	sockaddr_storage	addr;
	std::size_t			len;
	int					err;

	len = peer->to_sockaddr( addr );
	
	err			= ::connect( m_fd, ( struct sockaddr* ) &addr, len );
	would_block = ( err < 0 ) && ( ( platform::error() == ( int ) socket::error::in_progress ) || ( platform::error() == ( int ) socket::error::would_block ) ) ? true : false;

	return err;
}


int
socket::finish_connect()
{
	// We'll use a trick to determine if the connect worked as described here:
	//
	// http://cr.yp.to/docs/connect.html
	//
	
	sockaddr_storage	addr;
	socklen_t			len = sizeof( addr );
	int					err;
					
	err = getpeername( m_fd, ( struct sockaddr* ) &addr, &len );
	
	return err;
}

	
std::streamsize
socket::start_send( const std::uint8_t *buf, std::size_t len, bool &would_block )
{
	std::streamsize num;
	
	num			= ::send( m_fd, reinterpret_cast< const_buf_t >( buf ), len, 0 );
	would_block = ( num < 0 ) && ( platform::error() == ( int ) socket::error::would_block ) ? true : false;

	if ( ( num < 0 ) && ( !would_block ) )
	{
		nklog( log::error, "send returned %d", platform::error() );
	}
	
	return num;
}

	
std::streamsize
socket::start_recv( std::uint8_t *buf, std::size_t len, bool &would_block )
{
	std::streamsize num;

	num			= ::recv( m_fd, reinterpret_cast< buf_t >( buf ), len, 0 );
	would_block = ( num < 0 ) && ( platform::error() == ( int ) socket::error::would_block ) ? true : false;
	
	if ( ( num < 0 ) && ( !would_block ) )
	{
		nklog( log::error, "recv returned %d", platform::error() );
	}
	
	return num;
}


bool
socket::set_blocking( native fd, bool block )
{
#if defined( WIN32 )
	u_long flags = block ? 0 : 1;

	return ioctlsocket( fd, FIONBIO, &flags ) ? true : false;
#else
	int flags = block ? fcntl( fd, F_GETFL, 0 ) & ~O_NONBLOCK : fcntl( fd, F_GETFL, 0 ) | O_NONBLOCK;

	return fcntl( fd, F_SETFL, flags ) == 0 ? true : false;
#endif
}


void
socket::close()
{
	source::close();

	for ( auto it = m_close_handlers.begin(); it != m_close_handlers.end(); it++ )
	{
		it->second();
	}
	
	if ( m_fd != null )
	{
#if defined( WIN32 )
		::closesocket( m_fd );
#else
		::close( m_fd );
#endif
		m_fd = null;
	}
}


bool
socket::is_open() const
{
	return ( m_fd != null ) ? true : false;
}

#if defined( __APPLE__ )
#	pragma mark acceptor implementation
#endif

acceptor::acceptor( const endpoint::ptr &endpoint, int domain, int type )
:
	m_endpoint( endpoint )
{
	m_fd = ::socket( domain, type, 0 );
	socket::set_blocking( m_fd, false );
}


acceptor::acceptor( socket::native fd )
:
	m_fd( fd )
{
	socket::set_blocking( m_fd, false );
}


acceptor::~acceptor()
{
}


int
acceptor::listen( int size )
{
	sockaddr_storage	saddr	= { 0 };
	sockaddr_storage	saddr2	= { 0 };
	socklen_t			slen	= sizeof( sockaddr_storage );
	std::size_t			len		= 0;
	int					ret;
	
	len = m_endpoint->to_sockaddr( saddr );
	
	ret = ::bind( m_fd, ( struct sockaddr* ) &saddr, len );
	
	if ( ret != 0 )
	{
		nklog( log::error, "bind() failed: %d", platform::error() );
		goto exit;
	}
	
	ret = ::getsockname( m_fd, ( struct sockaddr* ) &saddr2, &slen );
	
	if ( ret != 0 )
	{
		nklog( log::error, "getsockname() failed: %d", platform::error() );
		goto exit;
	}
	
	m_endpoint = endpoint::from_sockaddr( saddr2 );
	
	ret = ::listen( m_fd, size );
	
	if ( ret != 0 )
	{
		nklog( log::error, "listen() failed: %d", platform::error() );
		goto exit;
	}
	
exit:

	return ret;
}
	
#if defined( __APPLE__ )
#	pragma mark ip::socket implementation
#endif

ip::socket::socket( int domain, int type )
:
	netkit::socket( domain, type )
{
}


ip::socket::socket( native fd )
:
	netkit::socket( fd )
{
}


ip::socket::socket( native fd, const ip::endpoint::ptr &peer )
:
	netkit::socket( fd )
{
	m_peer = peer;
}

#if defined( __APPLE__ )
#	pragma mark ip::acceptor implementation
#endif

ip::acceptor::acceptor( const ip::endpoint::ptr &endpoint, int type )
:
	netkit::acceptor( endpoint, endpoint->addr()->is_v4() ? AF_INET : AF_INET6, type )
{
}


ip::acceptor::acceptor( socket::native fd )
:
	netkit::acceptor( 0 )
{
}


#if defined( __APPLE__ )
#	pragma mark ip::tcp::socket implementation
#endif

ip::tcp::socket::socket()
:
	ip::socket( AF_INET, SOCK_STREAM )
{
}


ip::tcp::socket::socket( native fd )
:
	ip::socket( fd )
{
}


ip::tcp::socket::socket( native fd, const ip::endpoint::ptr &peer )
:
	ip::socket( fd, peer )
{
}


ip::tcp::socket::~socket()
{
}


/*
void
ip::tcp::socket::close()
{
	if ( m_connected )
	{
		::shutdown( m_fd, SHUT_RDWR );
		m_connected = false;
	}
	
	socket::close();
}
*/


	
#if defined( __APPLE__ )
#	pragma mark adapter implementation
#endif

ip::tcp::acceptor::acceptor( const ip::endpoint::ptr &endpoint )
:
	ip::acceptor( endpoint, SOCK_STREAM ),
	m_event( nullptr )
{
}


ip::tcp::acceptor::~acceptor()
{
	if ( m_event )
	{
		runloop::instance()->cancel( m_event );
	}
}


void
ip::tcp::acceptor::accept( accept_reply_f reply )
{
	assert( !m_event );

	if ( !m_event )
	{
		m_event = runloop::instance()->create( m_fd, runloop::event_mask::read );
	
		runloop::instance()->schedule( m_event, [=]( runloop::event event )
		{
			socket::native			new_fd;
			struct sockaddr_storage	addr;
			socklen_t				addr_len = sizeof( addr );
			socket::ptr				new_sock;
			
			runloop::instance()->cancel( m_event );
			m_event = nullptr;
		
			new_fd = ::accept( m_fd, ( struct sockaddr* ) &addr, &addr_len );
		
			if ( new_fd != socket::null )
			{
				new_sock = new ip::tcp::socket( new_fd, new ip::endpoint( addr ) );
				
			// new_sock->get_ethernet_addr();
			
				reply( 0, new_sock.get() );
			}
		} );
	}
}