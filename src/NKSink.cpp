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
#include <NetKit/NKWebSocket.h>
#include <NetKit/NKTLS.h>
#include <NetKit/NKLog.h>

using namespace netkit;

sink::sink()
{
}


sink::~sink()
{
}


void
sink::bind( source::ref source )
{
	m_source = source;

	m_on_close = m_source->on_close( std::bind( &sink::source_was_closed, this ) );
	
	run();
}


void
sink::unbind()
{
	m_on_close	= nullptr;
	m_source	= nullptr;

	source_was_closed();
}


cookie::ref
sink::on_close( close_f c )
{
	static std::uint64_t	tags	= 0;
	std::uint64_t			t		= ++tags;
	cookie::ref				cookie;
	
	m_close_handlers.push_back( std::make_pair( t, c ) );
	
	cookie.reset( reinterpret_cast< void* >( t ), [=]( void *v )
	{
		for ( auto it = m_close_handlers.begin(); it != m_close_handlers.end(); it++ )
		{
			if ( it->first == t )
			{
				m_close_handlers.erase( it );
				break;
			}
		}
	} );
	
	return cookie;
}

	
void
sink::source_was_closed()
{
	// This can get kind of tricky. It's very possible for whatever code
	// executes in these close_handlers to cause this sink to be deleted.
	// 
	// That is a bad thing. So to prevent it, we'll artifically bump up
	// our reference count, and then release right after.

	sink::ref artifical( this );

	for ( auto it = m_close_handlers.begin(); it != m_close_handlers.end(); it++ )
	{
		it->second();
	}
}


void
sink::connect( const uri::ref &uri, source::connect_reply_f reply )
{
	source::ref source = new ip::tcp::socket;

	source->connect( uri, [=]( int status, const endpoint::ref &peer )
	{
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
	return m_source ? m_source->is_open() : false;
}


void
sink::close()
{
	m_source->close();
}


void
sink::run()
{
	sink::ref artifical( this );

	m_source->recv( [=]( int status, const std::uint8_t *buf, std::size_t len )
	{
		if ( artifical->m_source->is_open() )
		{
			if ( status == 0 )
			{
				if ( len > 0 )
				{
					if ( process( buf, len ) )
					{
						if ( is_open() )
						{
							run();
						}
					}
					else
					{
						nklog( log::verbose, "process() returned an error...closing connection", status );
						close();
					}
				}
			}
			else if ( status == -2 )
			{
				nklog( log::verbose, "connection closed" );
				close();
			}
			else
			{
				nklog( log::verbose, "source::recv() returned an error (%d)...closing connection", status );
				close();
			}
		}
	} );
}


endpoint::ref
sink::peer() const
{
	return m_source->peer();
}
