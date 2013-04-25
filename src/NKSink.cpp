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
 
#include <NetKit/NKSink.h>
#include <NetKit/NKSource.h>
#include <NetKit/NKSocket.h>

using namespace netkit;

sink::sink()
{
}


sink::sink( const uri::ptr &uri )
{
	ip::tcp::socket::ptr sock = new ip::tcp::socket;
	
	sock->connect( uri, [=]( int status, const endpoint::ptr &endpoint )
	{
		if ( status == 0 )
		{
			m_source = sock.get();
		}
	} );
}


sink::~sink()
{
	fprintf( stderr, "in sink::~sink\n" );
}


void
sink::bind( source::ptr source )
{
	m_source = source;
	
	run();
}


tag
sink::bind( close_f c )
{
	return m_source->bind( c );
}

	
void
sink::unbind( tag t )
{
	if ( m_source.get() == t )
	{
		m_source = nullptr;
	}
	else
	{
		m_source->unbind( t );
	}
}


void
sink::connect( const uri::ptr &uri, source::connect_reply_f reply )
{
	source::ptr source = new ip::tcp::socket;

	fprintf( stderr, "before this = 0x%lx\n", this );
	source->connect( uri, [=]( int status, const endpoint::ptr &peer )
	{
		fprintf( stderr, "after this = 0x%lx\n", this );
		if ( status == 0 )
		{
			bind( source );
		}

		reply( status, peer );
	} );
}


void
sink::send( const std::uint8_t *buf, size_t len, source::send_reply_f reply )
{
	return m_source->send( buf, len, reply );
}


bool
sink::is_open() const
{
	return m_source->is_open();
}


void
sink::close()
{
	m_source->close();
}


void
sink::run()
{
	m_source->recv( m_buf, sizeof( m_buf ), [=]( int status, std::size_t len )
	{
		if ( len > 0 )
		{
			process( m_buf, len );
			
			run();
		}
		else
		{
			fprintf( stderr, "m_srouce->recv() returns %d\n", status );
		}
	} );
}