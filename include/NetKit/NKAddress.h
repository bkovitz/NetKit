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
#include <NetKit/NKExpected.h>
#if defined( WIN32 )
#	include <WinSock2.h>
#	include <Ws2tcpip.h>
#else
#	include <sys/socket.h>
#	include <arpa/inet.h>
#	include <net/if.h>
#	include <net/if_dl.h>
#	include <netinet/in.h>
#	include <netdb.h>
#endif
#include <vector>
#include <deque>


namespace netkit {

class NETKIT_DLL address : public object
{
public:

	typedef smart_ref< address > ref;
	
	static address::ref
	from_sockaddr( const sockaddr_storage &addr );

	virtual std::string
	to_string() const = 0;
};

namespace ip {

class address : public netkit::address
{
public:

	typedef smart_ref< address > ref;
	typedef std::deque< ref > list;
	typedef std::vector< ref > array;
	typedef std::function< void ( int status, const list &addrs ) > resolve_reply_f;
	
	enum type
	{
		v4 = 0,
		v6
	};
	
public:

	static void
	resolve( std::string host, resolve_reply_f reply );
	
	address( uint32_t addr );
	
	address( struct in_addr addr );
	
	address( struct in6_addr addr );
	
	address( const std::string &val );
	
	virtual ~address();

	inline std::int32_t
	type() const
	{
		return m_type;
	}
	
	inline bool
	is_v4() const
	{
		return ( m_type == v4 ) ? true : false;
	}
	
	inline bool
	is_v6() const
	{
		return ( m_type == v6 ) ? true : false;
	}
	
	expected< in_addr >
	to_v4() const;
	
	expected< in6_addr >
	to_v6() const;

	virtual std::string
	to_string() const;
	
	virtual bool
	equals( const object &that ) const;

protected:

	std::int32_t m_type;
	
	union
	{
		in_addr		m_v4;
		in6_addr	m_v6;
	} m_addr;
};

}

}

#endif
