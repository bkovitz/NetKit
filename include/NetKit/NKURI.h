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
 
#ifndef _netkit_uri_h
#define _netkit_uri_h

#include <NetKit/NKObject.h>
#include <NetKit/NKSmartRef.h>

#include <string>

namespace netkit {

class NETKIT_DLL uri : public object
{
public:

	typedef smart_ref< uri > ref;
	
	uri();
	
	uri( const std::string &scheme, const std::string& host, std::uint16_t port );
	
	uri( const std::string& s );

	~uri();
	
	static std::string
    escape( const std::string &s );

	inline const std::string&
	scheme() const
	{
		return m_scheme;
	}
	
	inline void
	set_scheme( const std::string &val )
	{
		m_scheme = val;
	}

	inline const std::string&
	host() const
	{
		return m_host;
	}
	
	inline void
	set_host( const std::string &val )
	{
		m_host = val;
	}

	inline int
	port() const
	{
		return m_port;
	}

	inline void
	set_port( int val )
	{
		m_port = val;
	}

	inline const std::string&
	path() const
	{
		return m_path;
	}

	inline void
	set_path( const std::string &val )
	{
		m_path = val;
	}

	inline const std::string&
	query() const
	{
		return m_query;
	}
	
	inline void
	set_query( const std::string &val )
	{
		m_query = val;
	}
	
	bool
	assign( const std::string &s );
	
	void
	clear();
	
	static std::string
	encode( const std::string &str );
	
	static std::string
	decode( const std::string& str );

	std::map< std::string, std::string >
	parameters() const;

	std::string
	to_string() const;

private:

	std::string	m_scheme;
	std::string	m_host;
	int			m_port;
	std::string	m_path;
	std::string	m_query;
};

}

#endif
