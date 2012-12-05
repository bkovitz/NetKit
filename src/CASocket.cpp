#include "socket.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

using namespace CoreApp;

socket::socket( int domain, int type )
:
	m_recv_handler( NULL ),
	m_send_handler( NULL ),
	m_recv_source( NULL )
{
	m_fd = ::socket( domain, type, 0 );
	set_blocking( false );
}


socket::socket( native fd )
:
	m_fd( fd ),
	m_recv_handler( NULL ),
	m_send_handler( NULL ),
	m_recv_source( NULL )
{
	set_blocking( false );
}


socket::~socket()
{
	close();
}


void
socket::close()
{
	if ( m_recv_source )
	{
		dispatch_release( m_recv_source );
		m_recv_source = NULL;
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


void
socket::set_send_handler( handler h )
{
}

	
void
socket::set_recv_handler( handler h )
{
	if ( !m_recv_source && ( m_fd != null ) )
	{
		m_recv_source = dispatch_source_create( DISPATCH_SOURCE_TYPE_READ, m_fd, 0, dispatch_get_main_queue() );
		
		dispatch_source_set_event_handler( m_recv_source,  ^( void )
		{
			m_recv_handler();
		} );
    
		dispatch_source_set_cancel_handler( m_recv_source,  ^( void )
		{
			//close(my_file);
		} );
    
		dispatch_resume( m_recv_source );
	}
	
	m_recv_handler = h;
}


int
socket::set_blocking( bool block )
{
	if ( block )
	{
#if defined( WIN32 )

		u_long iMode = 0;

		return ioctlsocket( m_fd, FIONBIO, &iMode );
#else

		return fcntl( m_fd, F_SETFL, fcntl( m_fd, F_GETFL, 0 ) & ~O_NONBLOCK );
#endif
	}
	else
	{
#if defined( WIN32 )

		u_long iMode = 1;

		return ioctlsocket( m_fd, FIONBIO, &iMode );
#else

		return fcntl( m_fd, F_SETFL, fcntl( m_fd, F_GETFL, 0 ) | O_NONBLOCK );
#endif
	}
}