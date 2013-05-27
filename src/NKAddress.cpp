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
 
#include <NetKit/NKAddress.h>
#include <NetKit/NKRunLoop.h>
#include <NetKit/NKPlatform.h>
#include <NetKit/NKLog.h>
#include <sstream>
#include <thread>

using namespace netkit;


inline bool
operator==( sockaddr_storage s1, sockaddr_storage s2 )
{
    return memcmp( &s1, &s2, sizeof( sockaddr_storage ) ) == 0;
}

#if defined( __APPLE__ )
#	pragma mark address implementation
#endif

address::ref
address::from_sockaddr( const sockaddr_storage &addr )
{
	address::ref ret;
	
	switch ( addr.ss_family )
	{
		case AF_INET:
		{
			ret = new ip::address( ( ( struct sockaddr_in* ) &addr )->sin_addr );
		}
		break;
		
		case AF_INET6:
		{
			ret = new ip::address( ( ( struct sockaddr_in6* ) &addr )->sin6_addr );
		}
		break;
	}
	
	return ret;
}

#if defined( __APPLE__ )
#	pragma mark ip::address implementation
#endif

ip::address::address( uint32_t addr )
{
	memset( &m_addr, 0, sizeof( m_addr ) );
	m_type				= v4;
	m_addr.m_v4.s_addr	= addr;
}


ip::address::address( struct in_addr addr )
{
	memset( &m_addr, 0, sizeof( m_addr ) );
	m_type		= v4;
	m_addr.m_v4	= addr;
}

	
ip::address::address( struct in6_addr addr )
{
	memset( &m_addr, 0, sizeof( m_addr ) );
	m_type		= v6;
	m_addr.m_v6	= addr;
}


ip::address::address( const std::string &val )
{
	memset( &m_addr, 0, sizeof( m_addr ) );
	
	if ( inet_pton( AF_INET, val.c_str(), &m_addr ) == 1 )
	{
		m_type = v4;
	}
	else if ( inet_pton( AF_INET6, val.c_str(), &m_addr ) == 1 )
	{
		m_type = v6;
	}
}


/*
address::address( addrinfo &ai )
{
	memset( &m_native, 0, sizeof( m_native ) );
	memcpy( &m_native, ai.ai_addr, ai.ai_addrlen );
	
	std::uint16_t port;
	char host[ 256 ];
	
	if ( m_native.ss_family == AF_INET )
	{
		sockaddr_in *saddr = ( struct sockaddr_in* ) &m_native;
		
		port = ntohs( saddr->sin_port );
		inet_ntop( AF_INET, &saddr->sin_addr, host, saddr->sin_len );
	}
	else if ( m_native.ss_family == AF_INET6 )
	{
		sockaddr_in6 *saddr = ( struct sockaddr_in6* ) &m_native;
		
		port = ntohs( saddr->sin6_port );
		inet_ntop( AF_INET6, &saddr->sin6_addr, host, saddr->sin6_len );
	}
}
*/


ip::address::~address()
{
}


void
ip::address::resolve( std::string host, resolve_reply_f reply )
{
	std::thread t( [=]()
	{
		address::list			addrs;
		struct addrinfo			*result;
		struct addrinfo			*res;
		std::ostringstream		os;
		int						err;
		
		err = getaddrinfo( host.c_str(), "0", NULL, &result );
    
		if ( err == 0 )
		{
			std::deque< struct sockaddr_storage > natives;

			for ( res = result; res != NULL; res = res->ai_next)
			{
				std::deque< struct sockaddr_storage >::iterator it;
				struct sockaddr_storage							storage;

				memset( &storage, 0, sizeof( storage ) );
				memcpy( &storage, res->ai_addr, res->ai_addrlen );
				
				it = std::find( natives.begin(), natives.end(), storage );
				
				if ( it == natives.end() )
				{
					if ( res->ai_family == AF_INET )
					{
						addrs.push_back( new address( ( ( struct sockaddr_in* ) res->ai_addr )->sin_addr ) );
					}
					else if ( res->ai_family == AF_INET6 )
					{
						addrs.push_back( new address( ( ( struct sockaddr_in6* ) res->ai_addr )->sin6_addr ) );
					}
					
					natives.push_back( storage );
				}
			}
 
			freeaddrinfo( result );
		}
		else
		{
			nklog( log::error, "error in getaddrinfo: %s", gai_strerror( err ) );
		}
		
		runloop::main()->dispatch( [=]()
		{
			reply( err, addrs );
		} );
	} );
	
	t.detach();
}


expected< in_addr >
ip::address::to_v4() const
{
	if ( is_v4() )
	{
		return m_addr.m_v4;
	}
	else
	{
		return std::runtime_error( "not an IPv4 address" );
	}
}


expected< in6_addr >
ip::address::to_v6() const
{
	if ( is_v6() )
	{
		return m_addr.m_v6;
	}
	else
	{
		return std::runtime_error( "not an IPv6 address" );
	}
}


std::string
ip::address::to_string() const
{
	char buf[ 1024 ];

	if ( is_v4() )
	{
#if defined( WIN32 )

		sockaddr_in	addr;
		DWORD		buf_size = sizeof( buf );

		memset( &addr, 0, sizeof( addr ) );
		addr.sin_family	= AF_INET;
		addr.sin_addr	= m_addr.m_v4;

		if ( WSAAddressToStringA( ( LPSOCKADDR ) &addr, sizeof( addr ), NULL, buf, &buf_size ) != 0 )
		
#else
	
		if ( inet_ntop( AF_INET, &m_addr.m_v4, buf, sizeof( buf ) ) == NULL )

#endif
		{
			nklog( log::error, "error converting addr to string: %d", platform::error() );
		}
	}
	else if ( is_v6() )
	{
#if defined( WIN32 )

		sockaddr_in6	addr;
		DWORD			buf_size = sizeof( buf );

		memset( &addr, 0, sizeof( addr ) );
		addr.sin6_family	= AF_INET6;
		addr.sin6_addr		= m_addr.m_v6;

		if ( WSAAddressToStringA( ( LPSOCKADDR ) &addr, sizeof( addr ), NULL, buf, &buf_size ) != 0 )

#else

		if ( inet_ntop( AF_INET6, &m_addr.m_v6, buf, sizeof( buf ) ) == NULL )
#endif
		{
		}
	}

	return buf;
}


bool
ip::address::equals( const object &that ) const
{
	bool ret = false;
	
	if ( this == &that )
	{
		ret = true;
	}
	else
	{
		const ip::address *actual = dynamic_cast< const ip::address* >( &that );
		
		if ( m_type == actual->m_type )
		{
			if ( m_type == v4 )
			{
				ret = memcmp( &m_addr.m_v4, &actual->m_addr.m_v4, sizeof( in_addr ) ) == 0 ? true : false;
			}
			else
			{
				ret = memcmp( &m_addr.m_v6, &actual->m_addr.m_v6, sizeof( in6_addr ) ) == 0 ? true : false;
			}
		}
	}
	
	return ret;
}