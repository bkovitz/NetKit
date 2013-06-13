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
 
#ifndef _netkit_proxy_h
#define _netkit_proxy_h

#include <NetKit/NKSocket.h>
#include <NetKit/NKURI.h>
#include <vector>
#include <string>

namespace netkit {

class NETKIT_DLL proxy : public object, public source::adapter
{
public:

	typedef std::function< bool ( smart_ref< proxy > proxy ) >	auth_challenge_f;
	typedef smart_ref< proxy >									ref;

	static proxy::ref
	null();

	static proxy::ref
	get();

	static void
	set( proxy::ref proxy );

	proxy();
	
	proxy( uri::ref uri );

	proxy( const json::value_ref &root );

	virtual ~proxy();

	inline const uri::ref&
	uri() const
	{
		return m_uri;
	}
	
	inline void
	bypass_add( const std::string &val )
	{
		auto it = std::find( m_bypass_list.begin(), m_bypass_list.end(), val );

		if ( it == m_bypass_list.end() )
		{
			m_bypass_list.push_back( val );
		}
	}

	inline const std::vector< std::string >&
	bypass_list() const
	{
		return m_bypass_list;
	}

	inline void
	bypass_remove( const std::string &val )
	{
		auto it = std::find( m_bypass_list.begin(), m_bypass_list.end(), val );

		if ( it != m_bypass_list.end() )
		{
			m_bypass_list.erase( it );
		}
	}

	bool
	bypass( const uri::ref &uri );

	void
	encode_authorization( const std::string &username, const std::string &password );

	inline const std::string&
	authorization() const
	{
		return m_authorization;
	}

	void
	decode_authorization( std::string &username, std::string &password ) const;

	inline void
	on_auth_challenge( auth_challenge_f handler )
	{
		m_auth_challenge_handler = handler;
	}

	inline bool
	auth_challenge()
	{
		bool ok = false;

		if ( m_auth_challenge_handler )
		{
			ok = m_auth_challenge_handler( this );
		}
		
		return ok; 
	}

	source::adapter::ref
	create( bool secure );

	virtual void
	flatten( json::value_ref &root ) const;

protected:

	void
	inflate( const json::value_ref &root );
	
	uri::ref					m_uri;
	std::vector< std::string >	m_bypass_list;
	auth_challenge_f			m_auth_challenge_handler;
	std::string					m_authorization;
};

}

#endif