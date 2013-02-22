#include <NetKit/NKSocket.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

using namespace netkit::socket;

server::server( int domain, int type )
{
	m_fd = ::socket( domain, type, 0 );
	//set_blocking( false );
}


server::server( native fd )
:
	m_fd( fd )
{
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


int
client::set_blocking( bool block )
{
#if defined( WIN32 )
	u_long flags = block ? 0 : 1;

	return ioctlsocket( m_fd, FIONBIO, &flags );
#else
	int flags = block ? fcntl( m_fd, F_GETFL, 0 ) & ~O_NONBLOCK : fcntl( m_fd, F_GETFL, 0 ) | O_NONBLOCK;

	return fcntl( m_fd, F_SETFL, flags );
#endif
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