#include "NKRunLoop_Win32.h"
#include <NetKit/NKStackWalk.h>
#include <NetKit/NKLog.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <process.h>
#include <list>
#include <sstream>
#include <thread>

#define DEBUG_RUNLOOP 1
/*
 * runloop_win32 Methods
 */

static runloop_win32 *g_instance;

using namespace netkit;

runloop::ref
runloop::main()
{
	return runloop_win32::main();
}


runloop_win32*
runloop_win32::main()
{
	if ( !g_instance )
	{
		g_instance = new runloop_win32;
		g_instance->retain();
	}

	return g_instance;
}


runloop_win32::runloop_win32()
:
	m_running( FALSE )
{
	init();
}


runloop_win32::~runloop_win32()
{
}


runloop::fd::ref
runloop_win32::create( std::int32_t domain, std::int32_t type, std::int32_t protocol )
{
	runloop::fd::ref	fd;
	SOCKET				s;

	s = WSASocket( domain, type, protocol, NULL, 0, WSA_FLAG_OVERLAPPED );
	
	if ( s == INVALID_SOCKET )
	{
		nklog( log::error, "WSASocket() failed: %d", WSAGetLastError() );
		goto exit;
	}
	
	fd = new fd_win32( s, domain, m_port );

exit:

	return fd;
}


runloop::fd::ref
runloop_win32::create( netkit::endpoint::ref in_endpoint, netkit::endpoint::ref &out_endpoint, std::int32_t domain, std::int32_t type, std::int32_t protocol )
{
	runloop::fd::ref	fd;
	sockaddr_storage	addr;
	int					len;
	SOCKET				s;

	s = WSASocket( domain, type, protocol, NULL, 0, WSA_FLAG_OVERLAPPED );
	
	if ( s == INVALID_SOCKET )
	{
		nklog( log::error, "WSASocket() failed: %d", WSAGetLastError() );
		goto exit;
	}

	len = ( int ) in_endpoint->to_sockaddr( addr );

	if ( ::bind( s, ( SOCKADDR* ) &addr, len ) != 0 )
	{
		nklog( log::error, "bind() failed: %d", ::WSAGetLastError() );
		goto exit;
	}

	if ( ::listen( s, SOMAXCONN ) != 0 )
	{
		nklog( log::error, "listen() failed: %d", ::WSAGetLastError() );
		goto exit;
	}

	len = sizeof( addr );

	if ( ::getsockname( s, ( sockaddr* ) &addr, &len ) != 0 )
	{
		nklog( log::error, "getsockname() failed: %d", ::WSAGetLastError() );
		goto exit;
	}

	out_endpoint = endpoint::from_sockaddr( addr );
	
	fd = new fd_win32( s, domain, m_port );

exit:

	return fd;
}


runloop::event
runloop_win32::create( HANDLE handle )
{
	source	*s = nullptr;
	
	try
	{
		s = new source;
	}
	catch ( ... )
	{
		s = nullptr;
	}

	if ( !s )
	{
		goto exit;
	}

	s->m_handle	= handle;

	m_sources.push_back( s );

exit:

	return s;
}


runloop::event
runloop_win32::create( std::time_t msec )
{
	source *s = nullptr;
	
	try
	{
		s = new source;
	}
	catch ( ... )
	{
		s = nullptr;
	}

	if ( !s )
	{
		goto exit;
	}

	s->m_relative_time	= msec;
	s->m_oneshot		= false;

exit:

	return s;
}


void
runloop_win32::schedule( event e, event_f func )
{
	source	*s			= reinterpret_cast< source* >( e );
	DWORD	registered	= FALSE;
	DWORD	err			= 0;

	assert( s );

	if ( !s )
	{
		nklog( log::error, "schedule() called with null event" );
		goto exit;
	}

	// First check our main Worker. In most cases, we won't have to worry about threads

	if ( s->m_scheduled )
	{
		if ( s->is_timer() )
		{
			nklog( log::error, "trying to schedule a timer event that has already been scheduled" );
		}
		else if ( s->is_handle() )
		{
			nklog( log::error, "trying to schedule an event that has already been scheduled with handle %d", s->m_handle );
		}

		goto exit;
	}

	s->m_scheduled	= true;
	s->m_func		= func;

	if ( s->is_timer() )
	{
		s->m_absolute_time = ( time( NULL ) * 1000 ) + s->m_relative_time;
		schedule( s );
	}
	else
	{
		if ( !s->m_scheduled )
		{
			if ( m_sources.size() < MAXIMUM_WAIT_OBJECTS )
			{
				schedule( s );
			}
			else
			{
				// Error
			}
		}
	}

exit:

	return;
}


void
runloop_win32::schedule_oneshot_timer( std::time_t msec, event_f func )
{
	source *s = reinterpret_cast< source* >( create( msec ) );

	s->m_oneshot = true;

	schedule( s, func );
}


void
runloop_win32::suspend( event e )
{
	source	*s = nullptr;

	assert( e );

	if ( !e )
	{
		nklog( log::error, "null event" );
		goto exit;
	}
	
	s = reinterpret_cast< source* >( e );

	if ( !s->m_scheduled )
	{
		nklog( log::warning, "trying to suspend an event that has not been scheduled" );
		goto exit;
	}

	s->m_scheduled = false;

	if ( s->is_handle() )
	{
		suspend( s );
	}

exit:

	return;
}

	
void
runloop_win32::cancel( event e )
{
	source	*s = nullptr;

	assert( e );

	if ( !e )
	{
		nklog( log::error, "null event" );
		goto exit;
	}

	s = reinterpret_cast< source* >( e );

	if ( s->m_scheduled )
	{
		suspend( s );
	}

	m_sources.erase( std::remove( m_sources.begin(), m_sources.end(), s ), m_sources.end() );

	// Don't delete the source now, because it holds the lambda context.  This will allow us
	// to call cancel while in the context of a lambda handler.  Otherwise, we'd be deleting
	// the context out from under us
	//
	// We'll perform a "neat" trick of delaying the delete until later...

	dispatch( [=]() mutable
	{
		delete s;
	} );

exit:

	return;
}

	
void
runloop_win32::dispatch( dispatch_f f )
{
	push( f );
}


void
runloop_win32::run( mode how )
{
	m_running = TRUE;

	do
	{
		bool input_event;

		run( how, input_event );

		if ( how == mode::once )
		{
			m_running = false;
		}
		else if ( input_event )
		{
			m_running = false;
		}
	}
	while ( m_running );
}


void
runloop_win32::stop()
{
	nklog( log::verbose, "" );
	m_running = FALSE;
}


runloop_win32::source::source()
:
	m_handle( WSA_INVALID_EVENT ),
	m_relative_time( 0 ),
	m_absolute_time( 0 ),
	m_scheduled( false )
{
}


runloop_win32::source::~source()
{
	if( m_handle != WSA_INVALID_EVENT )
	{
		WSACloseEvent( m_handle );
	}
}


void
runloop_win32::source::dispatch()
{
	m_func( this );
}


#if 0
runloop_win32::worker::worker()
:
	m_id( 0 ),
	m_wakeup( INVALID_HANDLE_VALUE ),
	m_thread( INVALID_HANDLE_VALUE ),
	m_done( FALSE ),
	m_result( 0 ),
	m_running( true )
{
}


runloop_win32::worker::~worker()
{
	if ( m_wakeup )
	{
		CloseHandle( m_wakeup );
		m_wakeup = INVALID_HANDLE_VALUE;
	}
}
#endif


bool
runloop_win32::init()
{
	source	*s;
	DWORD	err = 0;

	m_port = CreateIoCompletionPort( INVALID_HANDLE_VALUE , nullptr, NULL, 0 );

	if ( m_port == nullptr )
	{
		err = -1;
	}

	std::thread t( [=]()
	{
		while ( 1 )
		{
			DWORD				bytes_transferred;
			fd_win32			*fd;
			fd_win32::context	*context;
			BOOL				ok;

			ok = GetQueuedCompletionStatus( m_port, &bytes_transferred, ( PULONG_PTR ) &fd, ( LPOVERLAPPED* ) &context, INFINITE );

			if ( !ok )
			{
				nklog( log::error, "GetQueuedCompletionStatus failed: %d", ::GetLastError() );
			}

			if ( fd && context )
			{
				switch ( context->m_type )
				{
					case fd_win32::context::iocp_connect:
					{
						push( [=]()
						{
							fd->handle_connect( ok ? 0 : -1 );
						} );
					}
					break;

					case fd_win32::context::iocp_accept:
					{
						push( [=]()
						{
							fd->handle_accept( ok ? 0 : -1 );
						} );
					}
					break;

					case fd_win32::context::iocp_send:
					{
						push( [=]()
						{
							fd->handle_send( ok ? 0 : -1, reinterpret_cast< fd_win32::send_context* >( context ) );
						} );
					}
					break;

					case fd_win32::context::iocp_recv:
					{
						push( [=]()
						{
							fd->handle_recv( ok ? 0 : -1, bytes_transferred );
						} );
					}
					break;

					case fd_win32::context::iocp_recvfrom:
					{
						push( [=]()
						{
							fd->handle_recvfrom( ok ? 0 : -1, bytes_transferred );
						} );
					}
					break;
				}
			}
		}
	} );

	t.detach();

	try
	{
		s = new source;
	}
	catch ( ... )
	{
		s = nullptr;
	}

	if ( !s )
	{
		err = -1;
	}
	
	m_wakeup = CreateEvent( nullptr, FALSE, FALSE, nullptr );

	schedule( create( m_wakeup ), [=]( event e )
	{
		std::pair< void*, dispatch_f > item;

		while ( m_queue.try_pop( item ) )
		{
			item.second();
		}
	} );

	return ( err == 0 ) ? true : false;
}


void
runloop_win32::schedule( source *s )
{
	s->m_scheduled	= true;

	if ( !s->is_timer() )
	{
		m_sources.push_back( s );
	}
	else
	{
		m_timers.push_back( s );

		m_timers.sort( &source::compare );
	}
}


void
runloop_win32::suspend( source *s )
{
	if ( s->is_timer() )
	{
		m_timers.remove( s );
	}
	else
	{
		m_sources.erase( std::remove( m_sources.begin(), m_sources.end(), s ), m_sources.end() );
	}

	s->m_scheduled = false;
}


void
runloop_win32::run( mode how, bool &input_event )
{
	HANDLE		handles[ MAXIMUM_WAIT_OBJECTS ];
	std::size_t	index = 0;
	DWORD		result;
	DWORD		err		= 0;
	DWORD		timeout = INFINITE;

	input_event = false;

	if ( m_timers.size() > 0 )
	{
		std::time_t now = ( time( NULL ) * 1000 );

		while ( m_timers.size() )
		{
			source *s = m_timers.front();

			if ( now < s->m_absolute_time )
			{
				timeout = ( DWORD ) ( s->m_absolute_time - now );
				break;
			}
			else
			{
				m_timers.pop_front();
				s->m_scheduled = false;
				s->dispatch();

				if ( s->m_oneshot )
				{
					runloop::main()->cancel( s );
				}
			}
		}
	}

	for ( auto it = m_sources.begin(); it != m_sources.end(); it++ )
	{
		handles[ index++ ] = ( *it )->m_handle;
	}

	result = MsgWaitForMultipleObjects( ( DWORD ) m_sources.size(), handles, FALSE, timeout, ( how == mode::input_events ) ? QS_ALLEVENTS : 0 );

	if ( result == WAIT_FAILED )
	{
		err = GetLastError();
	}

	if ( err )
	{
		nklog( log::error, "WaitForMultipleObjects() returned error: %d", err );

		goto exit;
	}

	if ( result == WAIT_TIMEOUT )
	{
		if ( m_timers.size() )
		{
			auto s = m_timers.front();
			m_timers.pop_front();
			s->m_scheduled	= false;
			s->dispatch();
		}
	}
	else if ( result == WAIT_OBJECT_0 + m_sources.size() )
	{
		input_event = true;
	}
	else
	{
		DWORD	waitItemIndex = ( DWORD )( ( ( int ) result ) - WAIT_OBJECT_0 );
		source	*s = nullptr;
		
		// Sanity check

		if ( waitItemIndex >= m_sources.size() )
		{
			nklog( log::error, "waitItemIndex (%d) is >= numSources (%d)", waitItemIndex, m_sources.size() );
			goto exit;
		}

		s = m_sources[ waitItemIndex ];

		s->dispatch();
	}

exit:

	return;
}

LPFN_CONNECTEX				runloop_win32::fd_win32::m_connect_ex;
LPFN_ACCEPTEX				runloop_win32::fd_win32::m_accept_ex;
LPFN_GETACCEPTEXSOCKADDRS	runloop_win32::fd_win32::m_get_accept_sockaddrs;

runloop_win32::fd_win32::fd_win32( SOCKET fd, int domain, HANDLE port )
:
	m_domain( domain ),
	m_port( port ),
	m_fd( fd )
{
	assert( m_fd != INVALID_SOCKET );

	m_in_buf.resize( 8192 );

	if ( CreateIoCompletionPort( ( HANDLE ) m_fd, m_port, ( ULONG_PTR ) this, 0 ) == NULL )
	{
		nklog( log::error, "CreateIoCompletionPort() failed: %d", ::GetLastError() );
	}

	if ( !m_connect_ex || !m_accept_ex || !m_get_accept_sockaddrs )
	{
		SOCKET sock = socket( AF_INET, SOCK_STREAM, 0 );

		if ( sock != INVALID_SOCKET )
		{
			DWORD	dwBytes;
			int		rc;

			if ( !m_connect_ex )
			{
				GUID guid = WSAID_CONNECTEX;

				rc = WSAIoctl( sock, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof( guid ), &m_connect_ex, sizeof( m_connect_ex ), &dwBytes, NULL, NULL );

				if ( rc != 0 )
				{
				}
			}

			if ( !m_accept_ex )
			{
				GUID guid = WSAID_ACCEPTEX;

				rc = WSAIoctl( sock, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof( guid ), &m_accept_ex, sizeof ( m_accept_ex ), &dwBytes, NULL, NULL );

				if ( rc != 0 )
				{
				}
			}

			if ( !m_get_accept_sockaddrs )
			{
				GUID guid = WSAID_GETACCEPTEXSOCKADDRS;

				rc = WSAIoctl( sock, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof( guid ), &m_get_accept_sockaddrs, sizeof ( m_get_accept_sockaddrs ), &dwBytes, NULL, NULL );

				if ( rc != 0 )
				{
				}
			}

			::closesocket( sock );
		}
    }
}


runloop_win32::fd_win32::~fd_win32()
{
	nklog( log::verbose, "" );

	if ( m_fd != INVALID_SOCKET )
	{
		closesocket( m_fd );
	}
}


int
runloop_win32::fd_win32::bind( netkit::endpoint::ref to )
{
	struct sockaddr_storage addr;
	std::size_t				len;

	len = to->to_sockaddr( addr );

	auto ret = ::bind( m_fd, ( SOCKADDR* ) &addr, ( int ) len );

	if ( ret != 0 )
	{
		nklog( log::error, "bind() failed: %d", ::WSAGetLastError() );
	}

	return ret;
}


void
runloop_win32::fd_win32::connect( endpoint::ref to, connect_reply_f reply )
{
	sockaddr_storage	this_addr;
	sockaddr_storage	that_addr;
	std::size_t			that_addr_len;
	int					ret;
	BOOL				ok;

	ZeroMemory( &that_addr, sizeof( that_addr ) );
	that_addr_len = to->to_sockaddr( that_addr );

	ZeroMemory( &this_addr, sizeof( this_addr ) );
	this_addr.ss_family = that_addr.ss_family;

	ret = ::bind( m_fd, ( SOCKADDR* ) &this_addr, sizeof( this_addr ) );

	if ( ret != 0 )
	{
		nklog( log::error, "bind() failed: %d", ::GetLastError() );
		reply( -1, nullptr );
		goto exit;
    }

	assert( !m_connect_context.m_reply );
	m_connect_context.m_reply	= reply;
	m_connect_context.m_to		= to;

	ok = m_connect_ex( m_fd, ( SOCKADDR* ) &that_addr, ( int ) that_addr_len, NULL, 0, NULL, &m_connect_context );

	if ( ok || ( WSAGetLastError() == ERROR_IO_PENDING ) )
	{
		retain();
	}
	else
	{
		nklog( log::error, "ConnectEx() failed: %d", ::GetLastError() );
		reply( -1, nullptr );
    }
 
exit:

	return;
}


void
runloop_win32::fd_win32::handle_connect( int status )
{
	if ( m_fd != INVALID_SOCKET )
	{
		if ( status == 0 )
		{
			auto ret = setsockopt( m_fd, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0 );

			if ( ret != 0 )
			{
				nklog( log::warning, "setsockopt() failed: %d", ::GetLastError() );
			}
		}

		auto reply					= m_connect_context.m_reply;
		m_connect_context.m_reply	= nullptr;

		reply( status, m_connect_context.m_to );
	}

	release();
}


void
runloop_win32::fd_win32::accept( accept_reply_f reply )
{
	DWORD	dwBytes;
	BOOL	ok;

	m_accept_context.m_fd = WSASocket( m_domain, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED );

	if ( m_accept_context.m_fd == INVALID_SOCKET )
	{
		nklog( log::error, "WSASocket() failed: %d", ::GetLastError() );
		reply( -1, nullptr, nullptr );
		goto exit;
	}

	assert( !m_accept_context.m_reply );
	m_accept_context.m_reply = reply;

	ok = m_accept_ex( m_fd, m_accept_context.m_fd, &m_in_buf[ 0 ], 0, sizeof( sockaddr_storage ) + 16, sizeof( sockaddr_storage ) + 16, &dwBytes, &m_accept_context );

	if ( ok || ( ::GetLastError() == ERROR_IO_PENDING ) )
	{
		retain();
	}
	else
	{
		nklog( log::error, "AcceptEx() failed: %d", ::GetLastError() );
		reply( -1, nullptr, nullptr );
	}

exit:

	return;
}


void
runloop_win32::fd_win32::handle_accept( int status )
{
	if ( m_fd != INVALID_SOCKET )
	{
		netkit::endpoint::ref	from;
		fd_win32::ref			fd;

		if ( status == 0 )
		{
			endpoint::ref		to;
			sockaddr_storage	*to_addr;
			int					to_addr_len = 0;
			sockaddr_storage	*from_addr;
			int					from_addr_len = 0;
	
			auto ret = setsockopt( m_accept_context.m_fd, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, ( char* ) &m_accept_context.m_fd, sizeof( m_accept_context.m_fd ) );
	
			if ( ret != 0 )
			{
				nklog( log::warning, "setsockopt() failed: %d", ::GetLastError() );
			}

			m_get_accept_sockaddrs( &m_in_buf[ 0 ], 0, sizeof( sockaddr_storage ) + 16, sizeof( sockaddr_storage ) + 16, ( SOCKADDR** ) &to_addr, &to_addr_len, ( SOCKADDR** ) &from_addr, &from_addr_len );

			to		= netkit::endpoint::from_sockaddr( *to_addr );
			from	= netkit::endpoint::from_sockaddr( *from_addr );
	
			fd		= new fd_win32( m_accept_context.m_fd, m_domain, m_port );
		}

		auto reply					= m_accept_context.m_reply;
		m_accept_context.m_reply	= nullptr;

		reply( status, fd, from );
	}

	release();
}


void
runloop_win32::fd_win32::send( const std::uint8_t *buf, std::size_t len, send_reply_f reply )
{
	WSABUF	bufs[ 1 ];
	DWORD	bytes_sent;
	DWORD	err;

	bufs[ 0 ].buf = ( char* ) buf;
	bufs[ 0 ].len = ( ULONG ) len;

	// We can have multiple send's inflight, so we need to allocate
	// a unique context here

	auto context		= new send_context;
	context->m_reply	= reply;

	err = WSASend( m_fd, bufs, 1, &bytes_sent, 0, context, NULL );

	if ( !err || ( ::GetLastError() == ERROR_IO_PENDING ) )
	{
		retain();
	}
	else
	{
		nklog( log::error, "WSASend failed: %d", ::GetLastError() );
		reply( -1 );
	}
}


void
runloop_win32::fd_win32::sendto( const std::uint8_t *buf, std::size_t len, netkit::endpoint::ref to, send_reply_f reply )
{
	struct sockaddr_storage addr;
	std::size_t				addr_len;
	WSABUF					bufs[ 1 ];
	DWORD					bytes_sent;
	DWORD					err;

	bufs[ 0 ].buf = ( char* ) buf;
	bufs[ 0 ].len = ( ULONG ) len;

	// We can have multiple send's inflight, so we need to allocate
	// a unique context here

	auto context		= new send_context;
	context->m_reply	= reply;

	addr_len = to->to_sockaddr( addr );

	err = WSASendTo( m_fd, bufs, 1, &bytes_sent, 0, ( SOCKADDR* ) &addr, ( int ) addr_len, context, NULL );

	if ( !err || ( ::GetLastError() == ERROR_IO_PENDING ) )
	{
		retain();
	}
	else
	{
		nklog( log::error, "WSASend failed: %d", ::GetLastError() );
		reply( -1 );
	}
}


void
runloop_win32::fd_win32::handle_send( int status, send_context *context )
{
	assert( context );

	if ( m_fd != INVALID_SOCKET )
	{
		context->m_reply( status );
		delete context;
	}

	release();
}


void
runloop_win32::fd_win32::recv( recv_reply_f reply )
{
	WSABUF	bufs[ 1 ];
	DWORD	bytes_read;
	DWORD	err;

	bufs[ 0 ].buf = ( char* ) &m_in_buf[ 0 ];
	bufs[ 0 ].len = ( ULONG ) m_in_buf.size();

	assert( !m_recv_context.m_reply );
	m_recv_context.m_reply = reply;
	m_recv_context.m_flags = 0;

	err = WSARecv( m_fd, bufs, 1, &bytes_read, &m_recv_context.m_flags, &m_recv_context, nullptr );

	if ( !err || ( ::GetLastError() == ERROR_IO_PENDING ) )
	{
		retain();
	}
	else
	{
		nklog( log::error, "WSARecv failed: %d", ::GetLastError() );
		reply( -1, nullptr, 0 );
	}
}


void
runloop_win32::fd_win32::handle_recv( int status, DWORD bytes_read )
{
	if ( m_fd != INVALID_SOCKET )
	{
		auto reply				= m_recv_context.m_reply;
		m_recv_context.m_reply	= nullptr;

		reply( 0, &m_in_buf[ 0 ], bytes_read );
	}

	release();
}


void
runloop_win32::fd_win32::recvfrom( recvfrom_reply_f reply )
{
	WSABUF	bufs[ 1 ];
	DWORD	bytes_read;
	DWORD	flags = 0;
	DWORD	err;

	bufs[ 0 ].buf = ( char* ) &m_in_buf[ 0 ];
	bufs[ 0 ].len = ( ULONG ) m_in_buf.size();

	assert( !m_recvfrom_context.m_reply );
	m_recvfrom_context.m_reply		= reply;
	m_recvfrom_context.m_flags		= 0;
	m_recvfrom_context.m_addr_len	= sizeof( m_recvfrom_context.m_addr );

	err = WSARecvFrom( m_fd, bufs, 1, &bytes_read, &m_recvfrom_context.m_flags, ( SOCKADDR* ) &m_recvfrom_context.m_addr, &m_recvfrom_context.m_addr_len, &m_recvfrom_context, nullptr );

	if ( !err || ( ::GetLastError() == ERROR_IO_PENDING ) )
	{
		retain();
	}
	else
	{
		nklog( log::error, "WSARecvFrom failed: %d", ::GetLastError() );
		m_recvfrom_context.m_reply = nullptr;
		reply( -1, nullptr, 0, nullptr );
	}
}


void
runloop_win32::fd_win32::handle_recvfrom( int status, DWORD bytes_read )
{
	if ( m_fd != INVALID_SOCKET )
	{
		endpoint::ref from			= endpoint::from_sockaddr( m_recvfrom_context.m_addr );
		auto reply					= m_recvfrom_context.m_reply;
		m_recvfrom_context.m_reply	= nullptr;

		reply( 0, &m_in_buf[ 0 ], bytes_read, from );
	}

	release();
}


void
runloop_win32::fd_win32::close()
{
	if ( m_fd != INVALID_SOCKET )
	{
		nklog( log::verbose, "sock = %d", m_fd );
		::closesocket( m_fd );
		m_fd = INVALID_SOCKET;
	}
}
