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