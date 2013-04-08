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
 
#include <NetKit/NKIPAddress.h>
#include <NetKit/NKRunLoop.h>
#include <NetKit/NKPlatform.h>
#include <NetKit/NKLog.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sstream>
#include <thread>

using namespace netkit::ip;


inline bool
operator==( sockaddr_storage s1, sockaddr_storage s2 )
{
    return ( s1.ss_len == s2.ss_len ) && ( memcmp( &s1, &s2, s1.ss_len ) == 0 );
}


address::address( uint32_t addr, uint16_t port )
{
	sockaddr_in *saddr = ( struct sockaddr_in* ) &m_native;
	
	memset( &m_native, 0, sizeof( m_native ) );
	
	saddr->sin_family = AF_INET;
	saddr->sin_len = sizeof( sockaddr_in );
	saddr->sin_addr.s_addr = addr;
	saddr->sin_port = htons( port );
}


address::address( struct in_addr addr, uint16_t port )
{
	sockaddr_in *saddr = ( struct sockaddr_in* ) &m_native;
	
	memset( &m_native, 0, sizeof( m_native ) );
	
	saddr->sin_family = AF_INET;
	saddr->sin_len = sizeof( sockaddr_in );
	saddr->sin_addr = addr;
	saddr->sin_port = htons( port );
}

	
address::address( struct in6_addr addr, uint16_t port )
{
	sockaddr_in6 *saddr = ( struct sockaddr_in6* ) &m_native;
	
	memset( &m_native, 0, sizeof( m_native ) );
	
	saddr->sin6_family = AF_INET6;
	saddr->sin6_len = sizeof( sockaddr_in6 );
	saddr->sin6_addr = addr;
	saddr->sin6_port = htons( port );
}


address::address( struct sockaddr_storage addr )
:
	m_native( addr )
{
}


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


address::~address()
{
}


void
address::resolve( std::string host, uint16_t port, resolve_reply_f reply )
{
	std::thread t( [=]()
	{
		address::list			addrs;
		struct addrinfo			*result;
		struct addrinfo			*res;
		struct addrinfo			hints;
		std::ostringstream		os;
		int						err;
		
		memset( &hints, 0, sizeof( hints ) );

		hints.ai_socktype	= SOCK_STREAM;
		hints.ai_family		= AF_INET;
 
		os << port;
		
		err = getaddrinfo( host.c_str(), os.str().c_str(), NULL, &result );
    
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
					addrs.push_back( new address( storage ) );
					natives.push_back( storage );
				}
			}
 
			freeaddrinfo( result );
		}
		else
		{
			nklog( log::error, "error in getaddrinfo: %s", gai_strerror( err ) );
		}
		
		runloop::instance()->dispatch_on_main_thread( [=]()
		{
			reply( err, addrs );
		} );
	} );
	
	t.detach();
}


std::string
address::host() const
{
	char buf[ 1024 ];

	if ( m_native.ss_family == AF_INET )
	{
#if defined( WIN32 )

		if ( WSAAddressToStringA( ( LPSOCKADDR ) &m_native, sizeof( m_native ), NULL, buf, &bufSize ) != 0 )
		
#else
	
		if ( inet_ntop( m_native.ss_family, &( ( ( struct sockaddr_in *) &m_native )->sin_addr ), buf, sizeof( buf ) ) == NULL )

#endif
		{
			nklog( log::error, "error converting addr to string: %d", platform::error() );
		}
	}
	else if ( m_native.ss_family == AF_INET6 )
	{
#if defined( WIN32 )

		if ( WSAAddressToStringA( ( LPSOCKADDR ) &m_native, sizeof( m_native ), NULL, buf, &bufSize ) != 0 )

#else

		if ( inet_ntop( m_native.ss_family, &( ( ( struct sockaddr_in6* ) &m_native )->sin6_addr ), buf, sizeof( buf ) ) == NULL )
#endif
		{
		}
	}

	return buf;
}

	
uint16_t
address::port() const
{
	uint16_t port = 0;

	if ( m_native.ss_family == AF_INET )
	{
		port = ntohs( ( ( sockaddr_in* ) &m_native )->sin_port );
	}
	else if ( m_native.ss_family == AF_INET6 )
	{
		port = ntohs( ( ( sockaddr_in6* ) &m_native )->sin6_port );
	}

	return port;
}