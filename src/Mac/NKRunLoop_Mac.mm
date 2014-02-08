/*
 * Copyright (c) 2013, Porchdog Software Inc.
 * All rights reserved.
 *
 * Redistribution and use in event and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of event code must retain the above copyright notice, this
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

#include "NKRunLoop_Mac.h"
#include <CoreFoundation/CoreFoundation.h>
#include <NetKit/NKSocket.h>
#include <NetKit/NKLog.h>
#include <dispatch/dispatch.h>
#include <sys/errno.h>
#include <thread>

using namespace netkit;

runloop::ref
runloop::main()
{
	static runloop::ref singleton = new runloop_mac;
	
	return singleton;
}


runloop_mac::runloop_mac()
{
	// Make sure there is at least one thing added to runloop
	
	int fd = ::socket( AF_INET, SOCK_STREAM, 0 );
}


runloop_mac::~runloop_mac()
{
}


runloop::fd::ref
runloop_mac::create( std::int32_t domain, std::int32_t type, std::int32_t protocol )
{
	runloop::fd::ref    fd;
	int					s;

	s = ::socket( domain, type, protocol );

	if ( s == -1 )
    {
        nklog( log::error, "socket() failed: %", errno );
        goto exit;
    }

    fd = new fd_mac( s, domain );

exit:

    return fd;
}


runloop::fd::ref
runloop_mac::create( netkit::endpoint::ref in_endpoint, netkit::endpoint::ref &out_endpoint, std::int32_t domain, std::int32_t type, std::int32_t protocol )
{
	runloop::fd::ref	fd;
	sockaddr_storage	addr;
	socklen_t			len;
	int					s;

	s = ::socket( domain, type, protocol );

	if ( s == -1 )
    {
        nklog( log::error, "socket() failed: %", errno );
        goto exit;
    }

    len = ( int ) in_endpoint->to_sockaddr( addr );

	if ( ::bind( s, ( sockaddr* ) &addr, len ) != 0 )
    {
        nklog( log::error, "bind() failed: %", errno );
        goto exit;
    }

    if ( ::listen( s, SOMAXCONN ) != 0 )
    {
        nklog( log::error, "listen() failed: %", errno );
        goto exit;
    }

    len = sizeof( addr );

    if ( ::getsockname( s, ( sockaddr* ) &addr, &len ) != 0 )
    {
        nklog( log::error, "getsockname() failed: %", errno );
        goto exit;
    }

    out_endpoint = endpoint::from_sockaddr( addr );

    fd = new fd_mac( s, domain );

exit:

    return fd;
}


runloop::event
runloop_mac::create( std::time_t msec )
{
	auto event = dispatch_source_create( DISPATCH_SOURCE_TYPE_TIMER, 0, 0, dispatch_get_main_queue() );

	dispatch_source_set_timer( event, dispatch_time( DISPATCH_TIME_NOW, msec * NSEC_PER_MSEC ), msec * NSEC_PER_MSEC, 0 );

	return event;
}


void
runloop_mac::schedule( event e, event_f f )
{
	auto event = reinterpret_cast< dispatch_source_t >( e );
	
	dispatch_source_set_event_handler( event, ^()
	{
		f( event );
	} );
	
	dispatch_resume( event );
}


void
runloop_mac::schedule_oneshot_timer( std::time_t msec, event_f func )
{
	auto event = dispatch_source_create( DISPATCH_SOURCE_TYPE_TIMER, 0, 0, dispatch_get_main_queue() );

	dispatch_source_set_timer( event, dispatch_time( DISPATCH_TIME_NOW, msec * NSEC_PER_MSEC ), DISPATCH_TIME_FOREVER, 0 );
	
	dispatch_source_set_event_handler( event, ^()
	{
		func( event );
		dispatch_source_cancel( event );
	} );
	
	dispatch_resume( event );
}


void
runloop_mac::suspend( event e )
{
	auto event = reinterpret_cast< dispatch_source_t >( e );
	dispatch_suspend( event );
}


void
runloop_mac::cancel( event e )
{
	auto event = reinterpret_cast< dispatch_source_t >( e );
	dispatch_source_cancel( event );
	dispatch_release( event );
}


void
runloop_mac::dispatch( dispatch_f f )
{
	dispatch_async( dispatch_get_main_queue(), ^()
	{
		f();
	} );
}


void
runloop_mac::run( mode how )
{
	CFRunLoopRun();
}

	
void
runloop_mac::stop()
{
	CFRunLoopStop( CFRunLoopGetCurrent() );
}


runloop_mac::fd_mac::fd_mac( int fd, int domain )
:
	m_domain( domain ),
	m_fd( fd ),
	m_send_source( dispatch_source_create( DISPATCH_SOURCE_TYPE_WRITE, fd, 0, dispatch_get_main_queue() ) ),
	m_recv_source( dispatch_source_create( DISPATCH_SOURCE_TYPE_READ, fd, 0, dispatch_get_main_queue() ) )
{
	assert( m_fd != -1 );
	assert( m_send_source );
	assert( m_recv_source );

	m_in_buf.resize( 8192 );
}


runloop_mac::fd_mac::~fd_mac()
{
	nklog( log::verbose, "" );
	
	close();
}


int
runloop_mac::fd_mac::bind( netkit::endpoint::ref to )
{
	struct sockaddr_storage addr;
	std::size_t				len;

	len = to->to_sockaddr( addr );

	auto ret = ::bind( m_fd, ( sockaddr* ) &addr, ( int ) len );

	if ( ret != 0 )
	{
		nklog( log::error, "bind() failed: %", errno );
	}

	return ret;
}


void
runloop_mac::fd_mac::connect( endpoint::ref to, connect_reply_f reply )
{
	sockaddr_storage	this_addr;
	sockaddr_storage	that_addr;
	socklen_t			that_addr_len;
	int					ret;

	memset( &that_addr, 0, sizeof( that_addr ) );
	that_addr_len = static_cast< socklen_t >( to->to_sockaddr( that_addr ) );

	memset( &this_addr, 0, sizeof( this_addr ) );
	this_addr.ss_family = that_addr.ss_family;
	
	ret = ::connect( m_fd, ( sockaddr* ) &that_addr, that_addr_len );
	
	if ( ret == 0 )
	{
		reply( 0, to );
	}
	else if ( errno == EWOULDBLOCK )
	{
		dispatch_source_set_event_handler( m_send_source, ^()
		{
			sockaddr_storage	addr;
			socklen_t			len;
			int					ret;
			
			suspend_send();
			
			memset( &addr, 0, sizeof( addr ) );
			len = sizeof( addr );
			
			ret = ::getpeername( m_fd, ( sockaddr* ) &addr, &len );
			
			if ( ret == 0 )
			{
				reply( 0, to );
			}
			else
			{
				nklog( log::error, "connect() failed" );
				reply( -1, nullptr );
			}
		} );
		
		resume_send();
	}
	else
	{
		nklog( log::error, "connect() failed: %", errno );
		reply( -1, nullptr );
    }
 
exit:

	return;
}


void
runloop_mac::fd_mac::accept( std::size_t peek, accept_reply_f reply )
{
	if ( m_fd != -1 )
	{
		dispatch_source_set_event_handler( m_recv_source, ^()
		{
			struct sockaddr_storage from_addr;
			socklen_t				from_len;
			int						sock;
			
			suspend_recv();
			
			memset( &from_addr, 0, sizeof( from_addr ) );
			from_len = sizeof( from_addr );
			
			sock = ::accept( m_fd, ( sockaddr* ) &from_addr, &from_len );
		
			if ( sock >= 0 )
			{
				auto from	= netkit::endpoint::from_sockaddr( from_addr );
				auto fd		= new fd_mac( sock, m_domain );
				int	status = 0;
				
				if ( peek > 0 )
				{
					auto context = std::make_shared< accept_context >( peek );
					
					std::thread t( [=]() mutable
					{
						fd_set read_fds;

						FD_ZERO( &read_fds );
						FD_SET( sock, &read_fds );

						auto num = ::select( ( int ) sock + 1, &read_fds, nullptr, nullptr, nullptr );

						if ( num > 0 )
						{
							num = ( int ) ::recv( sock, context->m_peek_buf.data(), context->m_peek_buf.size(), MSG_PEEK );

							if ( num > 0 )
							{
								context->m_peek_len = num;
								status				= 0;
							}
							else
							{
								nklog( log::error, "::recv() failed: %", errno );
								status = -1;
							}
						}
						else
						{
							nklog( log::error, "::select() failed: %", errno );
							status = -1;
						}

						runloop::main()->dispatch( [=]()
						{
							reply( status, fd, from, context->m_peek_buf.data(), context->m_peek_len );
						} );
					} );

					t.detach();
				
				}
				else
				{
					reply( 0, new fd_mac( sock, m_domain ), from, nullptr, 0 );
				}
			}
			else
			{
				nklog( log::error, "::accept() failed: %", errno );
				reply( -1, nullptr, nullptr, nullptr, 0 );
			}
		} );
		
		resume_recv();
	}
	else
	{
		nklog( log::error, "fd is invalid" );
		reply( -1, nullptr, nullptr, nullptr, 0 );
	}
}


void
runloop_mac::fd_mac::send( const std::uint8_t *buf, std::size_t len, send_reply_f reply )
{
	m_send_queue.push( std::make_shared< send_context >( buf, len, reply ) );
	
	if ( m_send_queue.size() == 1 )
	{
		try_send( [=]( send_context::ref &context ) -> ssize_t
		{
			return ::send( m_fd, context->m_buffer.data() + context->m_bytes_written, context->m_buffer.size() - context->m_bytes_written, 0 );
		} );
	}
}


void
runloop_mac::fd_mac::sendto( const std::uint8_t *buf, std::size_t len, netkit::endpoint::ref to, send_reply_f reply )
{
	m_send_queue.push( std::make_shared< send_context >( buf, len, to, reply ) );
	
	if ( m_send_queue.size() == 1 )
	{
		try_send( [=]( send_context::ref &context ) -> ssize_t
		{
			return ::sendto( m_fd, context->m_buffer.data() + context->m_bytes_written, context->m_buffer.size() - context->m_bytes_written, 0, ( sockaddr* ) &context->m_to, context->m_to_len );
		} );
	}
}


void
runloop_mac::fd_mac::try_send( send_f func )
{
	while ( !m_send_queue.empty() )
	{
		auto context = m_send_queue.top();
	
		while ( context->m_bytes_written < context->m_buffer.size() )
		{
			auto ret = func( context );
			
			if ( ret > 0 )
			{
				context->m_bytes_written += ret;
			}
			else if ( ret == 0 )
			{
				nklog( log::verbose, "::send/to() returns 0" );
				deliver_reply( context, -1 );
				goto exit;
			}
			else if ( errno == EWOULDBLOCK )
			{
				dispatch_source_set_event_handler( m_send_source, ^()
				{
					suspend_send();
	
					try_send( func );
				} );
		
				resume_send();
			}
			else
			{
				nklog( log::verbose, "::send/to() failed: %", errno );
				deliver_reply( context, -1 );
				goto exit;
			}
		}
			
		deliver_reply( context, 0 );
	}

exit:

	return;
}
	

void
runloop_mac::fd_mac::recv( recv_reply_f reply )
{
	dispatch_source_set_event_handler( m_recv_source, ^()
	{
		suspend_recv();
		
		auto ret = ::recv( m_fd, m_in_buf.data(), m_in_buf.size(), 0 );
		
		if ( ret > 0 )
		{
			reply( 0, m_in_buf.data(), ret );
		}
		else if ( ret == 0 )
		{
			reply( -1, nullptr, 0 );
		}
		else
		{
			reply( -1, nullptr, 0 );
		}
	} );
	
	resume_recv();
}


void
runloop_mac::fd_mac::recvfrom( recvfrom_reply_f reply )
{
	dispatch_source_set_event_handler( m_recv_source, ^()
	{
		sockaddr_storage			from_addr;
		socklen_t					from_len;
		std::vector< std::uint8_t > buffer( 8192, 0 );
		ssize_t						ret;
		
		suspend_recv();
		
		memset( &from_addr, 0, sizeof( from_addr ) );
		from_len = sizeof( from_addr );
		
		ret = ::recvfrom( m_fd, buffer.data(), buffer.size(), 0, ( sockaddr* ) &from_addr, &from_len );
		
		if ( ret > 0 )
		{
			endpoint::ref from = endpoint::from_sockaddr( from_addr );
			
			reply( 0, buffer.data(), ret, from );
		}
		else if ( ret == 0 )
		{
			reply( -1, nullptr, 0, nullptr );
		}
		else
		{
			reply( -1, nullptr, 0, nullptr );
		}
	} );
	
	resume_recv();
}


void
runloop_mac::fd_mac::close()
{
	if ( m_send_source && m_send_active )
	{
		dispatch_release( m_send_source );
		m_send_source = nullptr;
		m_send_active = false;
	}
	
	if ( m_recv_source && m_recv_active )
	{
		dispatch_release( m_recv_source );
		m_recv_source = nullptr;
		m_recv_active = false;
	}
	
	if ( m_fd != -1 )
	{
		nklog( log::verbose, "sock = %", m_fd );
		::close( m_fd );
		m_fd = -1;
	}
}