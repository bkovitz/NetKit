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
#include <NetKit/NKLog.h>

#include <iostream>

using namespace netkit;

#if defined( min )
#	undef min
#endif

source::source()
:
	m_send_event( nullptr ),
	m_recv_event( nullptr )
{
	add( new adapter );
}


source::~source()
{
	if ( m_send_event )
	{
		runloop::main()->cancel( m_send_event );
	}

	if ( m_recv_event )
	{
		runloop::main()->cancel( m_recv_event );
	}
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
source::connect( const uri::ref &in_uri, connect_reply_f reply )
{
	endpoint::ref to;
	
	m_adapters.tail()->preflight( in_uri, [=]( int status, const uri::ref &out_uri )
	{
		if ( status == 0 )
		{
			connect_internal_1( out_uri, reply );
		}
	} );
}


void
source::handle_resolve( ip::address::list addrs, const uri::ref &uri, connect_reply_f reply )
{
	assert( addrs.size() > 0 );

	ip::endpoint::ref	endpoint = new ip::endpoint( addrs.front(), uri->port() );
	bool				would_block;
	int					ret;

	ret = start_connect( endpoint.get(), would_block );

	if ( ret == 0 )
	{
		connect_internal_2( uri, endpoint.get(), reply );
	}
	else if ( would_block )
	{
		runloop::main()->schedule( m_send_event, [=]( runloop::event event ) mutable
		{
			int ret;

			runloop::main()->suspend( event );
				
			ret = finish_connect();

			if ( ret == 0 )
			{
				connect_internal_2( uri, endpoint.get(), reply );
			}
			else
			{
				close( false );

				if ( addrs.size() > 1 )
				{
					addrs.pop_front();
					handle_resolve( addrs, uri, reply );
				}
				else
				{
					reply( ret, endpoint.get() );
				}
			}
		} );
	}
	else
	{
		close( false );

		if ( addrs.size() > 1 )
		{
			addrs.pop_front();
			handle_resolve( addrs, uri, reply );
		}
		else
		{
			reply( ret, endpoint.get() );
		}
	}
}


void
source::connect_internal_1( const uri::ref &uri, connect_reply_f reply )
{
	ip::address::resolve( uri->host(), [=]( int status, ip::address::list addrs )
	{
		if ( status == 0 )
		{
			handle_resolve( addrs, uri, reply );
		}
		else
		{
			reply( status, nullptr );
		}
	} );
}


void
source::connect_internal_2( const uri::ref &uri, const endpoint::ref &to, connect_reply_f reply )
{
	m_adapters.head()->connect( uri, to, [=]( int status )
	{
		reply( status, to );
	} );
}


void
source::send( const std::uint8_t *in_buf, size_t in_len, send_reply_f reply )
{
	send( m_adapters.head(), in_buf, in_len, reply );
}


void
source::send( adapter *adapter, const std::uint8_t *in_buf, size_t in_len, send_reply_f reply )
{
	adapter->send( in_buf, in_len, [=]( int status, const std::uint8_t *out_buf, std::size_t out_len ) mutable
	{
		if ( out_len > 0 )
		{
			send_info *info = new send_info( out_buf, out_len, reply );
			
			m_send_queue.push( info );
		
			if ( m_send_queue.size() == 1 )
			{
				send_internal();
			}
		}
		else
		{
			reply( status );
		}
	} );
}


void
source::send_internal()
{
	while ( !m_send_queue.empty() )
	{
		bool			would_block;
		send_info		*info = m_send_queue.front();
		std::streamsize ret;
		
		ret = start_send( &info->m_buf[ info->m_idx ], info->m_buf.size() - info->m_idx, would_block );
		
		if ( ret > 0 )
		{
			info->m_idx += ( std::size_t ) ret;
		
			if ( info->m_idx == info->m_buf.size() )
			{
				m_send_queue.pop();
				info->m_reply( 0 );
				delete info;
			}
		}
		else if ( would_block )
		{
			runloop::main()->schedule( m_send_event, [=]( runloop::event event )
			{
				runloop::main()->suspend( m_send_event );
				send_internal();
			} );

			break;
		}
		else
		{
			break;
		}
	}
}


void
source::peek( std::uint8_t *buf, std::size_t len, recv_reply_f reply )
{
	if ( m_recv_buf.size() > 0 )
	{
		std::size_t min = std::min( m_recv_buf.size(), len );
		
		memcpy( buf, &m_recv_buf[ 0 ], min );
		
		reply( 0, min );
	}
	else
	{
		recv_internal( buf, len, true, reply );
	}
}

	
void
source::recv( std::uint8_t *buf, std::size_t len, recv_reply_f reply )
{
	if ( m_recv_buf.size() > 0 )
	{
		std::size_t min = std::min( m_recv_buf.size(), len );
		
		memcpy( buf, &m_recv_buf[ 0 ], min );
		
		if ( m_recv_buf.size() == min )
		{
			m_recv_buf.clear();
		}
		else
		{
			m_recv_buf.resize( m_recv_buf.size() - min );
		}
		
		reply( 0, min );
	}
	else
	{
		recv_internal( buf, len, false, reply );
	}
}


void
source::recv_internal( std::uint8_t *in_buf, std::size_t in_len, bool peek_flag, recv_reply_f reply )
{
	bool			would_block	= false;
	std::streamsize num			= 0;
	
	num = start_recv( in_buf, in_len, would_block );
	
	if ( num > 0 )
	{
		m_adapters.head()->recv( in_buf, num, [=]( int status, const std::uint8_t *out_buf, std::size_t out_len )
		{
			if ( status == 0 )
			{
				std::size_t min = std::min( out_len, in_len );
				
				if ( out_len )
				{
					memcpy( in_buf, out_buf, min );
				
					if ( peek_flag )
					{
						m_recv_buf.resize( min );
						memmove( &m_recv_buf[ 0 ], out_buf, min );
					}
	
					if ( out_len > in_len )
					{
						std::size_t old = m_recv_buf.size();
						m_recv_buf.resize( old + ( out_len - in_len ) );
						memcpy( &m_recv_buf[ old ], out_buf + in_len, out_len - in_len );
					}
					
					reply( 0, min );
				}
				else if ( peek_flag )
				{
					peek( in_buf, in_len, reply );
				}
				else
				{
					recv( in_buf, in_len, reply );
				}
			}
			else
			{
				reply( -1, 0 );
			}
		} );
	}
	else if ( num == 0 )
	{
		reply( -1, 0 );
	}
	else if ( would_block )
	{
		runloop::main()->schedule( m_recv_event, [=]( runloop::event event )
		{
			runloop::main()->suspend( event );
			recv_internal( in_buf, in_len, peek_flag, reply );
		} );
	}
	else
	{
		reply( -1, 0 );
	}
}


void
source::close( bool notify )
{
	if ( m_send_event )
	{
		runloop::main()->cancel( m_send_event );
		m_send_event = nullptr;
	}

	if ( m_recv_event )
	{
		runloop::main()->cancel( m_recv_event );
		m_recv_event = nullptr;
	}

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
source::adapter::preflight( const uri::ref &uri, preflight_reply_f reply )
{
	reply( 0, uri );
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
	reply( 0, in_buf, in_len );
}
