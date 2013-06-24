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

	proxy_adapter( proxy::ref proxy );
	
	virtual ~proxy_adapter();

	virtual void
	resolve( const uri::ref &uri, resolve_reply_f reply );
	
	virtual void
	send( const std::uint8_t *in_buf, std::size_t in_len, send_reply_f reply );
	
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
	set_connected( bool val );

	proxy::ref				m_proxy;
	bool					m_connected;
	std::queue< buffer* >	m_pending_send_list;
	netkit::uri::ref		m_uri;
};


class http_adapter : public proxy_adapter
{
public:

	http_adapter( proxy::ref proxy, bool secure );

	virtual ~http_adapter();
	
	virtual void
	connect( const uri::ref &uri, const endpoint::ref &to, connect_reply_f reply );
		
	virtual void
	recv( const std::uint8_t *in_buf, std::size_t in_len, recv_reply_f reply );
	
protected:

	void
	send_connect();

	std::uint16_t
	parse_server_handshake();
	
	std::string	m_handshake;
	bool		m_secure;
};


class socks4_adapter : public proxy_adapter
{
public:

	socks4_adapter( proxy::ref proxy );
	
	virtual ~socks4_adapter();
	
	virtual void
	connect( const uri::ref &uri, const endpoint::ref &to, connect_reply_f reply );
	
	virtual void
	recv( const std::uint8_t *in_buf, std::size_t in_len, recv_reply_f reply );
	
protected:

	void
	send_connect( endpoint::ref endpoint );
	
	std::vector< std::uint8_t >	m_handshake;
};


class socks5_adapter : public proxy_adapter
{
public:

	socks5_adapter( proxy::ref proxy );
	
	virtual ~socks5_adapter();
	
	virtual void
	connect( const uri::ref &uri, const endpoint::ref &to, connect_reply_f reply );
	
	virtual void
	recv( const std::uint8_t *in_buf, std::size_t in_len, recv_reply_f reply );
	
protected:

	enum state
	{
		waiting_for_opening_response			= 0x1,
		waiting_for_authentication_response		= 0x2,
		waiting_for_connect_response			= 0x3,
	};
		
	void
	start_negotiate();
	
	void
	send_auth();
	
	void
	send_connect();
	
	std::vector< std::uint8_t >	m_handshake;
	std::uint8_t				m_method;
	std::uint8_t				m_state;
	sockaddr_storage			m_dest;
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
	if ( g_proxy->uri() )
	{
		if ( g_proxy->uri()->scheme() == "http" )
		{
			return new http_adapter( g_proxy, secure );
		}
		else if ( g_proxy->uri()->scheme() == "socks4" )
		{
			return new socks4_adapter( g_proxy );
		}
		else if ( g_proxy->uri()->scheme() == "socks5" )
		{
			return new socks5_adapter( g_proxy );
		}
	}
	
	return nullptr;
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


bool
proxy::is_http() const
{
	return ( m_uri ) ? ( m_uri->scheme() == "http" ) : false;
}


bool
proxy::is_socks4() const
{
	return ( m_uri ) ? ( m_uri->scheme() == "socks4" ) : false;
}


bool
proxy::is_socks5() const
{
	return ( m_uri ) ? ( m_uri->scheme() == "socks5" ) : false;
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
			username = tmp.substr( 0, spot );
			password = tmp.substr( spot + 1, m_authorization.size() );
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

proxy_adapter::proxy_adapter( proxy::ref proxy )
:
	m_proxy( proxy ),
	m_connected( false )
{
}


proxy_adapter::~proxy_adapter()
{
}


void
proxy_adapter::resolve( const uri::ref &uri, resolve_reply_f reply )
{
	m_uri = uri;
	
	m_next->resolve( m_proxy->uri(), [=]( int status, const netkit::uri::ref &uri, ip::address::list addrs )
	{
		reply( status, uri, addrs );
	} );
}

		
void
proxy_adapter::send( const std::uint8_t *in_buf, std::size_t in_len, send_reply_f reply )
{
	if ( m_connected )
	{
		m_next->send( in_buf, in_len, reply );
	}
	else
	{
		buffer *buf = new buffer( reply, in_buf, in_len );

		m_pending_send_list.push( buf );
	}
}


void
proxy_adapter::set_connected( bool val )
{
	m_connected = val;
	
	if ( m_connected )
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
	}
}

#if defined( __APPLE__ )
#	pragma mark http_adapter implementation
#endif

http_adapter::http_adapter( proxy::ref proxy, bool secure )
:
	proxy_adapter( proxy ),
	m_secure( secure )
{
}


http_adapter::~http_adapter()
{
}


void
http_adapter::connect( const uri::ref &uri, const endpoint::ref &to, connect_reply_f reply )
{
	m_next->connect( uri, to, [=]( int status )
	{
		if ( status == 0 )
		{
			if ( m_secure )
			{
				send_connect();
			}
			else
			{
				m_connected = true;
			}
		}

		reply( status );
	} );
}


void
http_adapter::recv( const std::uint8_t *in_buf, std::size_t in_len, recv_reply_f reply )
{
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
							set_connected( true );
							
							out_buf += ( end + 4 );
							out_len -= ( end + 4 );
						}
						else if ( http::status::proxy_authentication )
						{
							if ( proxy::auth_challenge() )
							{
								send_connect();
								
								m_handshake.clear();
								
								status	= 0;
								out_buf	= nullptr;
								out_len	= 0;
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
}


void
http_adapter::send_connect()
{
	std::string			msg;
	std::ostringstream	os;

	os << "CONNECT " << m_uri->host() << ":" << m_uri->port() << " HTTP/1.1\r\n";
	os << "Host: " << m_uri->host() << ":" << m_uri->port() << "\r\n";

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
http_adapter::parse_server_handshake()
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

#if defined( __APPLE__ )
#	pragma mark socks4_adapter implementation
#endif

socks4_adapter::socks4_adapter( proxy::ref proxy )
:
	proxy_adapter( proxy )
{
}

	
socks4_adapter::~socks4_adapter()
{
}

	
void
socks4_adapter::connect( const uri::ref &uri, const endpoint::ref &to, connect_reply_f reply )
{
	m_next->connect( uri, to, [=]( int status )
	{
		if ( status == 0 )
		{
			ip::address::resolve( m_uri->host(), [=]( int status, ip::address::list addrs )
			{
				if ( status == 0 )
				{
					// Find an IPv4 address...we need to fix this at some point
					
					for ( auto it = addrs.begin(); it != addrs.end(); it++ )
					{
						if ( ( *it )->is_v4() )
						{
							ip::endpoint::ref endpoint = new ip::endpoint( *it, m_uri->port() );
							
							send_connect( endpoint );
							
							break;
						}
					}
				}
				
				reply( status );
			} );
		}
		else
		{
			reply( status );
		}	
	} );
}

	
void
socks4_adapter::recv( const std::uint8_t *in_buf, std::size_t in_len, recv_reply_f reply )
{
	m_next->recv( in_buf, in_len, [=]( int status, const std::uint8_t *out_buf, std::size_t out_len, bool more_coming )
	{
		if ( status == 0 )
		{
			if ( !m_connected && out_len )
			{
				std::copy( out_buf, out_buf + out_len, std::back_inserter( m_handshake ) );
				
				if ( m_handshake.size() >= 8 )
				{
					if ( m_handshake[ 0 ] == 0x00 )
					{
						if ( m_handshake[ 1 ] == 0x90 )
						{
							std::rotate( m_handshake.begin(), m_handshake.begin() + 8, m_handshake.end() );
							
							out_buf = &m_handshake[ 8 ];
							out_len = m_handshake.size() - 8;
											
							set_connected( true );
						}
						else
						{
							nklog( log::error, "expecting 0x90, received 0x%x", m_handshake[ 0 ] );
							
							status	= -1;
							out_buf = nullptr;
							out_len = 0;
						}
					}
					else
					{
						nklog( log::error, "expecting 0x00, received 0x%x", m_handshake[ 0 ] );
						
						status	= -1;
						out_buf = nullptr;
						out_len = 0;
					}
				}
			}
		}

		reply( status, out_buf, out_len, more_coming );
	} );
}


void
socks4_adapter::send_connect( endpoint::ref endpoint )
{
	sockaddr_in		addr;
	std::uint8_t	buf[ 9 ];
	
	endpoint->to_sockaddr( ( sockaddr_storage& ) addr );
	
	buf[ 0 ] = 0x04;
	buf[ 1 ] = 0x01;
	buf[ 2 ] = ( ( std::uint8_t* ) &addr.sin_port )[ 0 ];
	buf[ 3 ] = ( ( std::uint8_t* ) &addr.sin_port )[ 1 ];
	buf[ 4 ] = ( ( std::uint8_t* ) &addr.sin_addr.s_addr )[ 0 ];
	buf[ 5 ] = ( ( std::uint8_t* ) &addr.sin_addr.s_addr )[ 1 ];
	buf[ 6 ] = ( ( std::uint8_t* ) &addr.sin_addr.s_addr )[ 2 ];
	buf[ 7 ] = ( ( std::uint8_t* ) &addr.sin_addr.s_addr )[ 3 ];
	buf[ 8 ] = 0x00;
	
	m_source->send( m_next, buf, sizeof( buf ), [=]( int status )
	{
	} );
}


#if defined( __APPLE__ )
#	pragma mark socks5_adapter implementation
#endif

socks5_adapter::socks5_adapter( proxy::ref proxy )
:
	proxy_adapter( proxy )
{
}

	
socks5_adapter::~socks5_adapter()
{
}

	
void
socks5_adapter::connect( const uri::ref &uri, const endpoint::ref &to, connect_reply_f reply )
{
	m_next->connect( uri, to, [=]( int status )
	{
		if ( status == 0 )
		{
			ip::address::resolve( m_uri->host(), [=]( int status, ip::address::list addrs )
			{
				if ( status == 0 )
				{
					// Find an IPv4 address...we need to fix this at some point
					
					for ( auto it = addrs.begin(); it != addrs.end(); it++ )
					{
						if ( ( *it )->is_v4() )
						{
							ip::endpoint::ref endpoint = new ip::endpoint( *it, m_uri->port() );
							
							endpoint->to_sockaddr( m_dest );
							
							start_negotiate();
							
							break;
						}
					}
				}
				
				reply( status );
			} );
		}
		else
		{
			reply( status );
		}
	} );
}

	
void
socks5_adapter::recv( const std::uint8_t *in_buf, std::size_t in_len, recv_reply_f reply )
{
	m_next->recv( in_buf, in_len, [=]( int status, const std::uint8_t *out_buf, std::size_t out_len, bool more_coming )
	{
		if ( status == 0 )
		{
			if ( !m_connected && out_len )
			{
				switch ( m_state )
				{
					case waiting_for_opening_response:
					{
						std::copy( out_buf, out_buf + out_len, std::back_inserter( m_handshake ) );
						
						if ( m_handshake.size() == 2 )
						{
							if ( ( m_handshake[ 0 ] == 0x05 ) && ( m_handshake[ 1 ] == m_method ) )
							{
								m_handshake.clear();
								
								if ( m_method == 0x00 )
								{
									send_connect();
								}
								else
								{
									send_auth();
								}
							}
							else if ( ( m_method == 0x00 ) && ( m_handshake[ 1 ] == 0xff ) && proxy::auth_challenge() )
							{
								m_handshake.clear();
									
								start_negotiate();
										
								status = 0;
							}
							else
							{
								status = -1;
							}
						}
						
						out_buf = nullptr;
						out_len = 0;
					}
					break;
					
					case waiting_for_authentication_response:
					{
						std::copy( out_buf, out_buf + out_len, std::back_inserter( m_handshake ) );
						
						if ( m_handshake.size() == 2 )
						{
							if ( ( m_handshake[ 0 ] == 0x01 ) && ( m_handshake[ 1 ] == 0x00 ) )
							{
								m_handshake.clear();
								
								send_connect();
							}
							else
							{
								status	= -1;
							}
						}
						
						out_buf = nullptr;
						out_len = 0;
					}
					break;
					
					case waiting_for_connect_response:
					{
						std::copy( out_buf, out_buf + out_len, std::back_inserter( m_handshake ) );
						
						if ( m_handshake.size() >= 4 )
						{
							if ( ( m_handshake[ 0 ] == 0x05 ) && ( m_handshake[ 1 ] == 0x00 ) )
							{
								switch ( m_handshake[ 3 ] )
								{
									case 0x01:
									{
										if ( m_handshake.size() >= 10 )
										{
											std::rotate( m_handshake.begin(), m_handshake.begin() + 10, m_handshake.end() );
											
											if ( m_handshake.size() > 0 )
											{
												out_buf = &m_handshake[ 10 ];
												out_len = m_handshake.size() - 10;
											}
											else
											{
												out_buf = nullptr;
												out_len = 0;
											}
											
											set_connected( true );
										}
										else
										{
											out_buf = nullptr;
											out_len = 0;
										}
									}
									break;
									
									case 0x03:
									{
										if ( m_handshake.size() >= 5 )
										{
											std::uint8_t len = m_handshake[ 4 ];
											
											if ( m_handshake.size() >= ( 5 + len ) )
											{
												std::rotate( m_handshake.begin(), m_handshake.begin() + 5 + len, m_handshake.end() );
												
												if ( m_handshake.size() > 0 )
												{
													out_buf = &m_handshake[ 5 + len ];
													out_len = m_handshake.size() - 5 + len;
												}
												else
												{
													out_buf = nullptr;
													out_len = 0;
												}
												
												set_connected( true );
											}
											else
											{
												out_buf = nullptr;
												out_len = 0;
											}
										}
										else
										{
											out_buf = nullptr;
											out_len = 0;
										}
									}
									break;
									
									case 0x04:
									{
										if ( m_handshake.size() >= 22 )
										{
											std::rotate( m_handshake.begin(), m_handshake.begin() + 22, m_handshake.end() );
											
											if ( m_handshake.size() > 0 )
											{
												out_buf = &m_handshake[ 22 ];
												out_len = m_handshake.size() - 22;
											}
											else
											{
												out_buf = nullptr;
												out_len = 0;
											}
												
											set_connected( true );
										}
										else
										{
											out_buf = nullptr;
											out_len = 0;
										}
									}
									break;
									
									default:
									{
										status	= -1;
										out_buf = nullptr;
										out_len = 0;
									}
									break;
								}
							}
							else
							{
								status	= -1;
								out_buf	= nullptr;
								out_len	= 0;
							}
						}
						else
						{
							out_buf = nullptr;
							out_len = 0;
						}
					}
					break;
				}
			}
		}

		reply( status, out_buf, out_len, more_coming );
	} );
}


void
socks5_adapter::start_negotiate()
{
	std::uint8_t buf[ 3 ];
	
	buf[ 0 ] = 0x05;
	buf[ 1 ] = 0x01;
	m_method = ( m_proxy->authorization().size() > 0 ) ? 0x02 : 0x00;
	buf[ 2 ] = m_method;
	
	m_source->send( m_next, buf, sizeof( buf ), [=]( int status )
	{
	} );
	
	m_state = waiting_for_opening_response;
}


void
socks5_adapter::send_auth()
{
	std::string		username;
	std::string		password;
	std::uint8_t	ulen;
	std::uint8_t	plen;
	std::uint8_t	buf[ 1024 ];	// this is bigger than we can use, so no buffer overflows
	
	m_proxy->decode_authorization( username, password );
	
	ulen = username.size() > 255 ? 255 : ( std::uint8_t ) username.size();
	plen = password.size() > 255 ? 255 : ( std::uint8_t ) password.size();
	
	buf[ 0 ] = 0x01;
	buf[ 1 ] = ulen;
	memcpy( buf + 2, username.c_str(), ulen );
	buf[ 2 + ulen ] = plen;
	memcpy( buf + 3 + ulen, password.c_str(), plen );
	
	m_source->send( m_next, buf, ulen + plen + 3, [=]( int status )
	{
	} );
	
	m_state = waiting_for_authentication_response;
}

	
void
socks5_adapter::send_connect()
{
	std::uint8_t	buf[ 1024 ];
	std::size_t		len = 0;
	
	buf[ len++ ] = 0x05;
	buf[ len++ ] = 0x01;
	buf[ len++ ] = 0x00;
	
	if ( m_dest.ss_family == AF_INET )
	{
		sockaddr_in *destv4 = ( sockaddr_in* ) &m_dest;
		
		buf[ len++ ] = 0x01;
		memcpy( buf + len, &destv4->sin_addr.s_addr, sizeof( destv4->sin_addr.s_addr ) );
		len += sizeof( destv4->sin_addr.s_addr );
		buf[ len++ ] = ( ( std::uint8_t* ) &destv4->sin_port )[ 0 ];
		buf[ len++ ] = ( ( std::uint8_t* ) &destv4->sin_port )[ 1 ];
	}
	else if ( m_dest.ss_family == AF_INET6 )
	{
		sockaddr_in6 *destv6 = ( sockaddr_in6* ) &m_dest;
		
		buf[ len++ ] = 0x04;
		memcpy( buf + len, &destv6->sin6_addr, sizeof( destv6->sin6_addr ) );
		len += sizeof( destv6->sin6_addr );
		buf[ len++ ] = ( ( std::uint8_t* ) &destv6->sin6_port )[ 0 ];
		buf[ len++ ] = ( ( std::uint8_t* ) &destv6->sin6_port )[ 1 ];
	}
	
	m_source->send( m_next, buf, len, [=]( int status )
	{
	} );
	
	m_state = waiting_for_connect_response;
}