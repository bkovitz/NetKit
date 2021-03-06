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

#ifndef _netkit_oauth_h
#define _netkit_oauth_h

#include <NetKit/NKError.h>
#include <cstdint>
#include <chrono>
#include <string>
#include <functional>
#include <stack>

namespace netkit {

class oauth
{
public:

	struct token
	{
		std::chrono::system_clock::time_point	expire_time;
		std::string								access_token;
		std::string								refresh_token;
	};

	typedef std::function< void( netkit::status status, bool token_changed, const token &token ) > access_token_reply_f;

	oauth( const std::string &client_id, const std::string &client_secret, const std::string &auth_server_uri );

	oauth( const std::string &client_id, const std::string &client_secret, const std::string &auth_server_uri, const std::string &refresh_token );

	oauth( const std::string &client_id, const std::string &client_secret, const std::string &auth_server_uri, const std::string &refresh_token, const std::string &access_token, const std::chrono::system_clock::time_point &expire_time );

	void
	access_token( access_token_reply_f reply );

	inline struct token&
	token()
	{
		return m_token;
	}

private:

	std::stack< access_token_reply_f >	m_update_queue;
	std::string							m_client_id;
	std::string							m_client_secret;
	std::string							m_redirect_uri;
	std::string							m_auth_server_uri;
	struct token						m_token;
};

}

#endif