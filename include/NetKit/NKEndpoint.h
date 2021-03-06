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
 */
 
#ifndef _netkit_endpoint_h
#define _netkit_endpoint_h

#include <NetKit/NKURI.h>
#include <NetKit/NKAddress.h>
#include <deque>

namespace netkit {

class NETKIT_DLL endpoint : public object
{
public:

	typedef smart_ref< endpoint > ref;
	
	static endpoint::ref
	from_sockaddr( const sockaddr_storage &addr );

	virtual std::size_t
	to_sockaddr( sockaddr_storage &addr ) const = 0;
	
	virtual std::string
	to_string() const = 0;
};

namespace ip {

class NETKIT_DLL endpoint : public netkit::endpoint
{
public:

	typedef smart_ref< endpoint > ref;
	typedef std::deque< ref > list;
	
public:

	endpoint( addrinfo &ai );
	
	endpoint( const sockaddr_storage &addr );
	
	endpoint( const uri::ref &uri );
	
	endpoint( int domain, std::uint16_t port );

	endpoint( const address::ref &host, std::uint16_t port );

	endpoint( const netkit::json::value_ref &json );
	
	virtual ~endpoint();
	
	virtual std::size_t
	to_sockaddr( sockaddr_storage &addr ) const;
	
	virtual std::string
	to_string() const;
	
	inline const address::ref&
	addr() const
	{
		return m_addr;
	}
	
	inline std::uint16_t
	port() const
	{
		return m_port;
	}
	
	virtual bool
	equals( const object &that ) const;

	virtual bool
	equals( const ip::endpoint::ref &that ) const;

	virtual void
	flatten( json::value_ref &root ) const;

	void
	inflate( const json::value_ref &root );

protected:

	address::ref	m_addr;
	std::uint16_t	m_port;
};

}

}

#endif
