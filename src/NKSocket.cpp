#include <NetKit/NKSocket.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

using namespace netkit::socket;

static int
set_blocking( native fd, bool block )
{
#if defined( WIN32 )
	u_long flags = block ? 0 : 1;

	return ioctlsocket( fd, FIONBIO, &flags );
#else
	int flags = block ? fcntl( fd, F_GETFL, 0 ) & ~O_NONBLOCK : fcntl( fd, F_GETFL, 0 ) | O_NONBLOCK;

	return fcntl( fd, F_SETFL, flags );
#endif
}


server::server( int domain, int type )
{
	m_fd = ::socket( domain, type, 0 );
	set_blocking( false );
}


server::server( native fd )
:
	m_fd( fd )
{
}


int
server::set_blocking( bool block )
{
	return ::set_blocking( m_fd, block );
}


void
server::bind( std::initializer_list< adopt_f > l )
{
	m_adopters.assign( l.begin(), l.end() );
}
	
	
client::client( int domain, int type )
{
	m_fd = ::socket( domain, type, 0 );
	set_blocking( false );
}


client::client( native fd )
{
	m_fd = fd;
	set_blocking( false );
}


client::~client()
{
	close();
}


void
client::close()
{
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


int
client::set_blocking( bool block )
{
	return ::set_blocking( m_fd, block );
}


/*
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
	
}
*/