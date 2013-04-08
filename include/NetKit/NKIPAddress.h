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
 
#ifndef _netkit_ip_address_h
#define _netkit_ip_address_h

#include <NetKit/NKObject.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <deque>


namespace netkit {

namespace ip {

class address : public object
{
public:

	typedef smart_ptr< address > ptr;
	typedef std::deque< ptr > list;
	typedef std::function< void ( int status, const list &addrs ) > resolve_reply_f;
	
public:

	address( uint32_t addr, uint16_t );
	
	address( struct in_addr addr, uint16_t port );
	
	address( struct in6_addr addr, uint16_t port );
	
	address( sockaddr_storage sockaddr );
	
	address( addrinfo &ai );

	virtual ~address();
	
	static void
	resolve( std::string host, uint16_t port, resolve_reply_f reply );
	
	inline const sockaddr_storage&
	sockaddr() const
	{
		return m_native;
	}
	
	std::string
	host() const;
	
	uint16_t
	port() const;
	
	void
	set_port( uint16_t port )
	{
		if ( m_native.ss_family == AF_INET )
		{
			( ( struct sockaddr_in* ) &m_native )->sin_port = htons( port );
		}
		else if ( m_native.ss_family == AF_INET6 )
		{
			( ( struct sockaddr_in6* ) &m_native )->sin6_port = htons( port );
		}
	}
	
	inline bool
	operator==( const address::ptr &that ) const
	{
		return operator==( that.operator->() );
	}
	
	inline bool
	operator==( const address * that ) const
	{
		if ( !that )
		{
			return false;
		}
		else
		{
			return memcmp( &m_native, &that->m_native, sizeof( m_native ) ) == 0;
		}
	}

protected:

	struct sockaddr_storage m_native;
};

}
}

#endif
