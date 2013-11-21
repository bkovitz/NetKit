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
 
#include <NetKit/NKSource.h>
#include <NetKit/NKProxy.h>
#include <NetKit/NKTLS.h>
#include <NetKit/NKWebSocket.h>
#include <NetKit/NKLog.h>

#include <iostream>

using namespace netkit;

#if defined( min )
#	undef min
#endif

source::source()
:
	m_recv_buf( 8192 ),
	m_closed( false )
{
	add( new adapter );
}


source::~source()
{
	teardown_notifications();
}


void
source::add( adapter::ref adapter )
{
	if ( !adapter )
	{
		nklog( log::warning, "attempting to add a null adapter" );
		goto exit;
	}
	
	m_adapters.push_front( adapter );
	adapter->m_source = this;
	
exit:

	return;
}


cookie
source::on_close( close_f c )
{
	static int		tags	= 0;
	std::uint32_t	t		= ++tags;
	
	m_close_handlers.push_back( std::make_pair( t, c ) );
	
	return reinterpret_cast< void* >( t );
}


void
source::cancel( cookie c )
{
	for ( auto it = m_close_handlers.begin(); it != m_close_handlers.end(); it++ )
	{
		if ( it->first == reinterpret_cast< std::uint64_t >( c.get() ) )
		{
			m_close_handlers.erase( it );
			break;
		}
	}
}


void
source::connect( const uri::ref &uri, connect_reply_f reply )
{
	if ( ( uri->scheme() == "http" ) || ( uri->scheme() == "xmpp" ) || ( uri->scheme() == "ws" ) )
	{
		if ( !proxy::get()->bypass( uri ) )
		{
			add( proxy::get()->create_adapter( false ) );
		}
	}
	else if ( ( uri->scheme() == "https" ) || ( uri->scheme() == "xmpps" ) || ( uri->scheme() == "wss" ) )
	{
		if ( !proxy::get()->bypass( uri ) )
		{
			add( proxy::get()->create_adapter( true ) );
		}

		add( tls::client::create() );
	}

	if ( ( uri->scheme() == "ws" ) || ( uri->scheme() == "wss" ) )
	{
		add( ws::client::create() );
	}

	m_adapters.head()->resolve( uri, [=]( int status, const uri::ref &out_uri, ip::address::list addrs )
	{	
		if ( status == 0 )
		{
			handle_resolve( addrs, out_uri, reply );
		}
		else
		{
			reply( status, nullptr );
		}
	} );
}


void
source::handle_resolve( ip::address::list addrs, const uri::ref &uri, connect_reply_f reply )
{
	assert( addrs.size() > 0 );

	ip::endpoint::ref endpoint = new ip::endpoint( addrs.front(), uri->port() );

	start_connect( endpoint.get(), [=]( int status, const endpoint::ref peer ) mutable
	{
		if ( status == 0 )
		{
			connect_internal( uri, endpoint.get(), reply );
		}
		else
		{
			if ( addrs.size() > 1 )
			{
				addrs.pop_front();
				handle_resolve( addrs, uri, reply );
			}
			else
			{
				reply( status, endpoint.get() );
			}
		}
	} );
}


void
source::connect_internal( const uri::ref &uri, const endpoint::ref &to, connect_reply_f reply )
{
	if ( m_adapters.head() )
	{
		m_adapters.head()->connect( uri, to, [=]( int status )
		{
			reply( status, to );
		} );
	}
	else
	{
		reply( -1, to );
	}
}


void
source::send( const std::uint8_t *in_buf, size_t in_len, send_reply_f reply )
{
	if ( m_adapters.head() )
	{
		send( m_adapters.head(), in_buf, in_len, reply );
	}
	else
	{
		reply( -1 );
	}
}


void
source::send( adapter *adapter, const std::uint8_t *in_buf, size_t in_len, send_reply_f reply )
{
	if ( adapter )
	{
		adapter->send( in_buf, in_len, [=]( int status, const std::uint8_t *out_buf, std::size_t out_len ) mutable
		{
			if ( out_len > 0 )
			{
				send_info *info = new send_info( out_buf, out_len );
			
				start_send( &info->m_buf[ info->m_idx ], info->m_buf.size() - info->m_idx, [=]( int status )
				{
					reply( status );
					delete info;
				} );
			}
			else
			{
				reply( status );
			}
		} );
	}
	else
	{
		reply( -1 );
	}
}


void
source::recv( recv_reply_f reply )
{
	if ( m_adapters.head() )
	{
		if ( !m_recv_queue.empty() )
		{
			buf_t buf = m_recv_queue.front();
			m_recv_queue.pop();
			
			reply( 0, &buf[ 0 ], buf.size() );
		}
		else
		{
			recv_internal( reply );
		}
	}
	else
	{
		reply( -1, nullptr, 0 );
	}
}


void
source::recv_internal( recv_reply_f reply )
{
	bool			would_block	= false;
	std::streamsize num			= 0;

	start_recv( [=]( int status, const std::uint8_t *buf, std::size_t len )
	{
		if ( len > 0 )
		{
			m_adapters.head()->recv( buf, len, [=]( int status, const std::uint8_t *out_buf, std::size_t out_len, bool more_coming )
			{
				if ( status == 0 )
				{
					if ( out_len )
					{
						m_recv_queue.push( buf_t( out_buf, out_buf + out_len ) );
					}
	
					if ( !more_coming )
					{
						recv( reply );
					}
				}
				else
				{
					reply( status, nullptr, 0 );
				}
			} );
		}
		else if ( len == 0 )
		{
			reply( -2, nullptr, 0 );
		}
		else
		{
			reply( -1, nullptr, 0 );
		}
	} );
}


void
source::close( bool notify )
{
	m_closed = true;

	teardown_notifications();

	if ( m_close_handlers.size() > 0 )
	{
		if ( notify )
		{
			for ( auto it = m_close_handlers.begin(); it != m_close_handlers.end(); it++ )
			{
				it->second();
			}
		}

		m_close_handlers.clear();
	}

	if ( m_adapters.head() )
	{
		source::ref self( this );

		// This is kinda sketchy.  We must artificially bump up our ref count here
		// to prevent the deletion of an adapter to cause our destructor to be 
		// called.
		//
		// DO NOT MODIFY THIS CODE unless you know what you're doing.

		while ( m_adapters.head() )
		{
			adapter::ref adapter = ( adapter::ref ) m_adapters.head();
			m_adapters.remove( adapter );
			delete adapter;
		}
	}

	// DANGER DANGER DANGER

	// Do not do anything here that references the socket as it could have been deleted
	// out from under us when we deleted the adapters
}


endpoint::ref
source::peer() const
{
	return nullptr;
}


void
source::teardown_notifications()
{
}


source::adapter::adapter()
:
	m_prev( nullptr ),
	m_next( nullptr )
{
}


source::adapter::~adapter()
{
}


void
source::adapter::resolve( const uri::ref &uri, resolve_reply_f reply )
{
	ip::address::resolve( uri->host(), [=]( int status, ip::address::list addrs )
	{
		reply( status, uri, addrs );
	} );
}

	
void
source::adapter::connect( const uri::ref &uri, const endpoint::ref &to, connect_reply_f reply )
{
	reply( 0 );
}


void
source::adapter::send( const std::uint8_t *in_buf, std::size_t in_len, send_reply_f reply )
{
	reply( 0, in_buf, in_len );
}

		
void
source::adapter::recv( const std::uint8_t *in_buf, std::size_t in_len, recv_reply_f reply )
{
	reply( 0, in_buf, in_len, false );
}
