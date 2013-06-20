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
 
#include <NetKit/NKProxy.h>
#include <NetKit/NKHTTP.h>
#include <NetKit/NKJSON.h>
#include <NetKit/NKBase64.h>
#include <NetKit/NKLog.h>
#include <sstream>
#include <vector>

using namespace netkit;

class proxy_adapter : public netkit::source::adapter
{
public:

	proxy_adapter( proxy::ref proxy, bool secure );
	
	virtual ~proxy_adapter();

	virtual void
	resolve( const uri::ref &uri, resolve_reply_f reply );
	
	virtual void
	connect( const uri::ref &uri, const endpoint::ref &to, connect_reply_f reply );
		
	virtual void
	send( const std::uint8_t *in_buf, std::size_t in_len, send_reply_f reply );
		
	virtual void
	recv( const std::uint8_t *in_buf, std::size_t in_len, recv_reply_f reply );
	
protected:

	struct buffer
	{
		send_reply_f				m_reply;
		std::vector< std::uint8_t > m_data;

		inline buffer( send_reply_f reply, const std::uint8_t *data, std::size_t len )
		:
			m_reply( reply ),
			m_data( data, data + len )
		{
		}
	};

	void
	send_connect();

	std::uint16_t
	parse_server_handshake();

	proxy::ref				m_proxy;
	bool					m_secure;
	bool					m_connected;
	std::string				m_handshake;
	std::queue< buffer* >	m_pending_send_list;
};


#if defined( __APPLE__ )
#	pragma mark proxy implementation
#endif

static proxy::ref g_proxy;

proxy::auth_challenge_f		proxy::m_auth_challenge_handler;
proxy::set_handlers			proxy::m_set_handlers;
static std::uint8_t			*g_ptr = nullptr;

source::adapter::ref
proxy::create( bool secure )
{
	return new proxy_adapter( g_proxy, secure );
}

void
proxy::on_auth_challenge( auth_challenge_f handler )
{
	m_auth_challenge_handler = handler;
}


bool
proxy::auth_challenge()
{
	bool ok = false;

	if ( m_auth_challenge_handler )
	{
		ok = m_auth_challenge_handler();
	}
		
	return ok; 
}


proxy::ref
proxy::get()
{
	return ( g_proxy ) ? g_proxy : proxy::null();
}


netkit::cookie
proxy::on_set( set_f handler )
{
	netkit::cookie cookie( g_ptr++ );
	
	m_set_handlers.push_back( std::make_pair( cookie, handler ) );
	
	return cookie;
}


void
proxy::cancel( netkit::cookie cookie )
{
	for ( auto it = m_set_handlers.begin(); it != m_set_handlers.end(); it++ )
	{
		if ( it->first.get() == cookie.get() )
		{
			m_set_handlers.erase( it );
			break;
		}
	}
}


void
proxy::set( proxy::ref val )
{
	if ( !g_proxy || !g_proxy->equals( *val ) )
	{
		g_proxy = val;

		for ( auto it = m_set_handlers.begin(); it != m_set_handlers.end(); it++ )
		{
			( it->second )( g_proxy );
		}
	}
}


proxy::ref
proxy::null()
{
	static proxy::ref null;

	if ( !null )
	{
		null = new proxy;
	}

	return null;
}


proxy::proxy()
{
}


proxy::proxy( uri::ref uri )
:
	m_uri( uri )
{
}


proxy::proxy( const json::value::ref &json )
{
	inflate( json );
}


proxy::~proxy()
{
}


bool
proxy::is_null() const
{
	return ( m_uri ) ? false : true;
}


void
proxy::encode_authorization( const std::string &username, const std::string &password )
{
	m_authorization = codec::base64::encode( username + ":" + password );
}


void
proxy::decode_authorization( std::string &username, std::string &password ) const
{
	std::string tmp = codec::base64::decode( m_authorization );

	if ( tmp.size() > 0 )
	{
		std::size_t spot = tmp.find( ':' );

		if ( spot != std::string::npos )
		{
			username = m_authorization.substr( 0, spot - 1 );
			password = m_authorization.substr( spot + 1, m_authorization.size() );
		}
	}
}


bool
proxy::bypass( const uri::ref &uri )
{
	bool bypass = false;

	if ( !m_uri || ( uri->host() == "localhost" ) || ( uri->host() == "127.0.0.1" ) )
	{
		bypass = true;
	}

	return bypass;
}


void
proxy::flatten( json::value_ref &root ) const
{
	if ( m_uri )
	{
		root[ "uri" ] = m_uri->to_string();
	}
		
	root[ "authorization" ]	= m_authorization;

	for ( auto it = m_bypass_list.begin(); it != m_bypass_list.end(); it++ )
	{
		root[ "bypass" ]->append( *it );
	}
}


void
proxy::inflate( const json::value_ref &root )
{
	if ( root[ "uri" ]->is_string() )
	{
		m_uri = new netkit::uri( root[ "uri" ]->as_string() );
	}

	m_authorization	= root[ "authorization" ]->as_string();

	for ( auto i = 0; i < root[ "bypass" ]->size(); i++ )
	{
		m_bypass_list.push_back( root[ "bypass" ][ i ]->as_string() );
	}
}


bool
proxy::equals( const object &that ) const
{
	const proxy *rhs = reinterpret_cast< const proxy* >( &that );
	bool ok = false;

	if ( rhs )
	{
		if ( ( !m_uri && !rhs->m_uri ) ||
		     ( m_uri && rhs->m_uri && ( m_uri->to_string() == rhs->m_uri->to_string() ) ) )
		{
			ok = true;
		}
	}

	return ok;
}


#if defined( __APPLE__ )
#	pragma mark proxy_adapter implementation
#endif

proxy_adapter::proxy_adapter( proxy::ref proxy, bool secure )
:
	m_proxy( proxy ),
	m_secure( secure ),
	m_connected( false )
{
}


proxy_adapter::~proxy_adapter()
{
}


void
proxy_adapter::resolve( const uri::ref &uri, resolve_reply_f reply )
{
	nklog( log::verbose, "uri = %s", m_proxy->uri()->to_string().c_str() );

	m_next->resolve( uri, reply );

#if 0
	m_next->resolve( proxy::get()->uri(), [=]( int status, const netkit::uri::ref &uri, ip::address::list addrs )
	{
		reply( status, uri, addrs );
	} );
#endif
}

	
void
proxy_adapter::connect( const uri::ref &uri, const endpoint::ref &to, connect_reply_f reply )
{
	nklog( log::verbose, "uri = %s", m_proxy->uri()->to_string().c_str() );

	m_next->connect( uri, to, reply );

#if 0
	m_next->connect( uri, to, [=]( int status )
	{
		if ( ( status == 0 ) && m_secure )
		{
			m_uri = uri;

			send_connect();
		}

		reply( status );
	} );
#endif
} 
		
void
proxy_adapter::send( const std::uint8_t *in_buf, std::size_t in_len, send_reply_f reply )
{
	nklog( log::verbose, "uri = %s", m_proxy->uri()->to_string().c_str() );

	m_next->send( in_buf, in_len, reply );
#if 0
	if ( m_secure && m_connected )
	{
		m_next->send( in_buf, in_len, reply );
	}
	else
	{
		buffer *buf = new buffer( reply, in_buf, in_len );

		m_pending_send_list.push( buf );
	}
#endif
}

		
void
proxy_adapter::recv( const std::uint8_t *in_buf, std::size_t in_len, recv_reply_f reply )
{
	nklog( log::verbose, "uri = %s", m_proxy->uri()->to_string().c_str() );

	m_next->recv( in_buf, in_len, reply );

#if 0
	m_next->recv( in_buf, in_len, [=]( int status, const std::uint8_t *out_buf, std::size_t out_len, bool more_coming )
	{
		if ( m_secure && !m_connected )
		{
			if ( status == 0 )
			{
				if ( out_len )
				{
					m_handshake.append( out_buf, out_buf + out_len );
					
					auto end = m_handshake.find( "\r\n\r\n" );

					if ( end != std::string::npos )
					{
						auto code = parse_server_handshake();

						if ( code == http::status::ok )
						{
							while ( m_pending_send_list.size() > 0 )
							{
								buffer *b = m_pending_send_list.front();
									
								m_source->send( m_next, &b->m_data[ 0 ], b->m_data.size(), [=]( int status )
								{
								} );
	
								m_pending_send_list.pop();
							
								delete b;
							}
	
							out_buf += ( end + 4 );
							out_len -= ( end + 4 );
						}
						else if ( http::status::proxy_authentication )
						{
							if ( proxy::auth_challenge() )
							{
								send_connect();
							}
							else
							{
								status	= code;
								out_buf = nullptr;
								out_len	= 0;
							}
						}
						else
						{
							status	= code;
							out_buf = nullptr;
							out_len	= 0;
						}
					}
					else
					{
						out_buf = nullptr;
						out_len = 0;
					}
				}
			}
		}

		reply( status, out_buf, out_len, more_coming );
	} );
#endif
}


void
proxy_adapter::send_connect()
{
	std::string			msg;
	std::ostringstream	os;

	os << "CONNECT " << m_proxy->uri()->host() << ":" << m_proxy->uri()->port() << " HTTP/1.1\r\n";
	os << "Host: " << m_proxy->uri()->host() << "\r\n";

	if ( proxy::get()->authorization().size() > 0 )
	{
		os << "Proxy-Authorization: basic " << m_proxy->authorization() << "\r\n";
	}

	os << "\r\n";

	msg = os.str();

	m_source->send( m_next, ( const std::uint8_t* ) msg.c_str(), msg.size(), [=]( int status )
	{
		if ( status != 0 )
		{
			nklog( log::error, "send failed: %d", status );
		}
	} );
}


std::uint16_t
proxy_adapter::parse_server_handshake()
{
	std::uint16_t code = -1;

	// There is nothing about this that is pretty. We're brute forcing this bitch

	if ( ( m_handshake[ 0 ] == 'H' ) &&
	     ( m_handshake[ 1 ] == 'T' ) &&
		 ( m_handshake[ 2 ] == 'T' ) &&
		 ( m_handshake[ 3 ] == 'P' ) &&
		 ( m_handshake[ 4 ] == '/' ) &&
		 ( m_handshake[ 5 ] == '1' ) &&
		 ( m_handshake[ 6 ] == '.' ) &&
		 ( ( m_handshake[ 7 ] == '0' ) || ( m_handshake[ 7 ] == '1') ) &&
		 ( m_handshake[ 8 ] == ' ' ) )
	{
		char *end;

		auto ret = strtol( &m_handshake[ 9 ], &end, 0 );

		if ( ret && ( ret != LONG_MAX ) && ( ret != LONG_MIN ) )
		{
			code = ( std::uint16_t ) ret;
		}
	}

	return code;
}