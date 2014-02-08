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
 
#include <NetKit/NKEndpoint.h>
#include <NetKit/NKJSON.h>
#include <NetKit/NKPlatform.h>
#include <NetKit/NKLog.h>
#include <sstream>
#include <thread>

using namespace netkit;

#if defined( __APPLE__ )
#	pragma mark endpoint implementation
#endif

endpoint::ref
endpoint::from_sockaddr( const sockaddr_storage &addr )
{
	endpoint::ref ret;
	
	if ( ( addr.ss_family == AF_INET ) || ( addr.ss_family == AF_INET6 ) )
	{
		ret = new ip::endpoint( addr );
	}
	
	return ret;
}

#if defined( __APPLE__ )
#	pragma mark ip::endpoint implementation
#endif



ip::endpoint::endpoint( addrinfo &ai )
{
	if ( ai.ai_family == AF_INET )
	{
		sockaddr_in *v4 = ( sockaddr_in* ) ai.ai_addr;
		
		m_addr = new ip::address( v4->sin_addr );
		m_port = ntohs( v4->sin_port );
	}
	else if ( ai.ai_family == AF_INET6 )
	{
		sockaddr_in6 *v6 = ( sockaddr_in6* ) ai.ai_addr;
		
		m_addr = new ip::address( v6->sin6_addr );
		m_port = ntohs( v6->sin6_port );
	}
}

	
ip::endpoint::endpoint( const sockaddr_storage &addr )
{
	if ( addr.ss_family == AF_INET )
	{
		sockaddr_in *v4 = ( sockaddr_in* ) &addr;
		
		m_addr = new ip::address( v4->sin_addr );
		m_port = ntohs( v4->sin_port );
	}
	else if ( addr.ss_family == AF_INET6 )
	{
		sockaddr_in6 *v6 = ( sockaddr_in6* ) &addr;
		
		m_addr = new ip::address( v6->sin6_addr );
		m_port = ntohs( v6->sin6_port );
	}
}

	
ip::endpoint::endpoint( const uri::ref &uri )
{
}


ip::endpoint::endpoint( int domain, std::uint16_t port )
:
	m_port( port )
{
	switch ( domain )
	{
		case AF_INET:
		{
			m_addr = new ip::address( INADDR_ANY );
		}
		break;

		case AF_INET6:
		{
			m_addr = new ip::address( in6addr_any );
		}
		break;
	}
}

	
ip::endpoint::endpoint( const address::ref &addr, uint16_t port )
:
	m_addr( addr ),
	m_port( port )
{
}


ip::endpoint::endpoint( const json::value::ref &json )
{
	inflate( json );
}


ip::endpoint::~endpoint()
{
}


std::size_t
ip::endpoint::to_sockaddr( sockaddr_storage &addr ) const
{
	std::size_t len = 0;

	memset( &addr, 0, sizeof( addr ) );

	if ( m_addr->is_v4() )
	{
		struct sockaddr_in *v4_addr = ( struct sockaddr_in* ) &addr;
		
		v4_addr->sin_family		= AF_INET;
		v4_addr->sin_addr		= m_addr->to_v4().get();
		v4_addr->sin_port		= htons( m_port );

		len						= sizeof( sockaddr_in );
	}
	else if ( m_addr->is_v6() )
	{
		struct sockaddr_in6 *v6_addr = ( struct sockaddr_in6* ) &addr;
		
		v6_addr->sin6_family	= AF_INET6;
		v6_addr->sin6_addr		= m_addr->to_v6().get();
		v6_addr->sin6_port		= htons( m_port );

		len						= sizeof( sockaddr_in6 );
	}

	return len;
}


std::string
ip::endpoint::to_string() const
{
	char				buf[ 1024 ];
	sockaddr_storage	addr;

	to_sockaddr( addr );
	
	if ( m_addr->is_v4() )
	{
#if defined( WIN32 )

		DWORD buf_size = sizeof( buf );

		if ( WSAAddressToStringA( ( LPSOCKADDR ) &addr, sizeof( addr ), NULL, buf, &buf_size ) != 0 )
		
#else
	
		if ( inet_ntop( addr.ss_family, &( ( ( struct sockaddr_in *) &addr )->sin_addr ), buf, sizeof( buf ) ) == NULL )
#endif
		{
			nklog( log::error, "error converting IPv4 addr to string: %", platform::error() );
			goto exit;
		}
	}
	else if ( m_addr->is_v6() )
	{
#if defined( WIN32 )

		DWORD buf_size = sizeof( buf );
		
		if ( WSAAddressToStringA( ( LPSOCKADDR ) &addr, sizeof( addr ), NULL, buf, &buf_size ) != 0 )

#else

		if ( inet_ntop( addr.ss_family, &( ( ( struct sockaddr_in6* ) &addr )->sin6_addr ), buf, sizeof( buf ) ) == NULL )
#endif
		{
			nklog( log::error, "error converting IPv6 addr to string: %", platform::error() );
			goto exit;
		}
	}
	
exit:

	return buf;
}


bool
ip::endpoint::equals( const object &that ) const
{
	bool ret = false;
	
	if ( this == &that )
	{
		ret = true;
	}
	else
	{
		const ip::endpoint *actual = dynamic_cast< const ip::endpoint* >( &that );

		if ( actual )
		{
			ret = equals( *actual );
		}
	}
	
	return ret;
}


bool
ip::endpoint::equals( const ip::endpoint::ref &that ) const
{
	return m_addr->equals( *that->m_addr ) && ( m_port == that->m_port );
}


void
ip::endpoint::flatten( json::value_ref &root ) const
{
	root[ "address" ] = m_addr->to_string();
	root[ "port" ] = m_port;
}


void
ip::endpoint::inflate( const json::value_ref &root )
{
	m_addr = new ip::address( root[ "address" ]->as_string() );
	m_port = root[ "port" ]->as_uint16();
}
