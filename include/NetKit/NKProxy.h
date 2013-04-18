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
#include <string>

namespace netkit {

class proxy : public object
{
public:

	typedef smart_ptr< proxy > ptr;

	proxy( uri::ptr uri );

	virtual ~proxy();

	inline const uri::ptr&
	uri() const
	{
		return m_uri;
	}
	
	inline void
	set_uri( const uri::ptr &uri )
	{
		m_uri = uri;
	}

	inline const std::string&
	host() const
	{
		return m_host;
	}

	inline void
	set_host( const std::string &host )
	{
		m_host = host;
	}

	inline const std::string&
	bypass() const
	{
		return m_bypass;
	}

	inline void
	set_bypass( const std::string &bypass )
	{
		m_bypass = bypass;
	}

	inline std::uint16_t
	port() const
	{
		return m_port;
	}

	inline void
	set_port( std::uint16_t port )
	{
		m_port = port;
	}

	inline const std::string&
	user() const
	{
		return m_user;
	}

	inline void
	set_user( const std::string &user )
	{
		m_user = user;
	}

	inline const std::string&
	password() const
	{
		return m_password;
	}

	inline void
	set_password( const std::string &password )
	{
		m_password = password;
	}
	
private:

	uri::ptr		m_uri;
	std::string		m_host;
	std::string		m_bypass;
	std::uint16_t	m_port;
	std::string		m_user;
	std::string		m_password;
};

}

#endif