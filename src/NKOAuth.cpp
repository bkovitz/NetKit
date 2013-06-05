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

#include <NetKit/NKOAuth.h>
#include <NetKit/NKLog.h>
#include <NetKit/NKHTTP.h>
#include <NetKit/NKJSON.h>
#include <chrono>

using namespace netkit;
using namespace std::chrono;

oauth::oauth( const std::string &client_id, const std::string &client_secret, const std::string &auth_server_uri )
:
	m_client_id( client_id ),
	m_client_secret( client_secret ),
	m_auth_server_uri( auth_server_uri )
{
	m_token.refresh_token	= "";
	m_token.expire_time		= system_clock::now() - seconds( 1 );
}


oauth::oauth( const std::string &client_id, const std::string &client_secret, const std::string &auth_server_uri, const std::string &refresh_token )
:
	m_client_id( client_id ),
	m_client_secret( client_secret ),
	m_auth_server_uri( auth_server_uri )
{
	m_token.refresh_token	= refresh_token;
	m_token.expire_time		= system_clock::now() - seconds( 1 );
}


oauth::oauth( const std::string &client_id, const std::string &client_secret, const std::string &auth_server_uri, const std::string &refresh_token, const std::string &access_token, const std::chrono::system_clock::time_point &expire_time )
:
	m_client_id( client_id ),
	m_client_secret( client_secret ),
	m_auth_server_uri( auth_server_uri )
{
	m_token.refresh_token	= refresh_token;
	m_token.access_token	= access_token;
	m_token.expire_time		= expire_time;
}


void
oauth::get_access_token( token_result_f result )
{
	if ( m_token.expire_time < system_clock::now() )
	{
		http::request::ref request = new http::request( http::method::post, 1, 1, new uri( m_auth_server_uri ) );
		request->add_to_header( "Content-Type", std::string( "application/x-www-form-urlencoded" ) );

		*request << "client_id=" << m_client_id << "&client_secret=" << m_client_secret << "&refresh_token="
		         << m_token.refresh_token << "&grant_type=" << "refresh_token";

		http::client::send( request, [=]( http::response::ref response )
		{
			bool success = false;

			m_token.expire_time = system_clock::now();

			if ( response )
			{
				if ( response->status() == 200 )
				{
					json::value::ref root = new json::value();

					root->load_from_string( response->body() );

					if ( root->is_member( "access_token" ) && root[ "access_token" ]->is_string() )
					{
						m_token.access_token = root[ "access_token" ]->as_string();
		
						std::uint32_t seconds_until_expire = root[ "expires_in" ]->as_uint32();
						m_token.expire_time = system_clock::now() + seconds( seconds_until_expire );
		
						success = true;
					}
					else
					{
						nklog( log::error, "bad json format: %s", root->to_string().c_str() );
					}
				}
				else
				{
					nklog( log::error, "http error trying to refresh access token: %d", response->status() );
				}
			}
			else
			{
				nklog( log::error, "no response from %s", request->uri()->to_string().c_str() );
			}
	
			result( success, m_token );
		} );
	}
	else
	{
		result( true, m_token );
	}
}