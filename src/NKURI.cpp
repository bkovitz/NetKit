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
 
#include <NetKit/NKURI.h>
#include <NetKit/cstring.h>
#include <uriparser/Uri.h>
#include <string>
#include <algorithm>
#include <functional>
#include <vector>
#include <cctype>


typedef std::vector< std::string > strings;

static strings
split( const std::string &str, const char &delim )
{
	strings						tokens;
	std::string::const_iterator pos		= str.begin();
	std::string::const_iterator last	= str.begin();
	std::string					lastToken;

	while ( pos != str.end() )
	{
		last = pos;
		
        pos = find( pos, str.end(), delim );

		if ( pos != str.end() )
		{
			std::string token = std::string( last, pos );
			
			if ( token.length() > 0 )
			{
                tokens.push_back(token);
			}

			last = ++pos;
        }
    }

	lastToken = std::string( last, pos );
	
	if ( lastToken.length() > 0 )
	{
        tokens.push_back( lastToken );
	}

    return tokens;
}


using namespace netkit;


uri::uri()
{
}


uri::uri( const std::string &scheme, const std::string& host, std::uint16_t port )
:
	m_scheme( scheme ),
	m_host( host ),
	m_port( port )
{
}


uri::uri( const std::string& s )
:
	m_port( 0 )
{
	assign( s );
}


uri::~uri()
{
}


std::string
uri::escape( const std::string &in )
{
    char        out[ 1024 ];
    const char  *term;
    std::string ret;

    term = uriEscapeA( in.c_str(), out, false, false );
    ret = ( term != NULL ) ? out : in;

    return ret;
}


std::string
uri::recompose() const
{
	std::string	ret;
	UriUriStructA	uri;
	std::string		port;
	strings			components;
	strings			encodedComponents;
	char			*str	= NULL;
	int				len;
	
	memset( &uri, 0, sizeof( UriUriStructA ) );
	
	uri.scheme.first		= m_scheme.c_str();
	uri.scheme.afterLast	= m_scheme.c_str() + m_scheme.size();
	
	uri.hostText.first		= m_host.c_str();
	uri.hostText.afterLast	= m_host.c_str() + m_host.size();
	
	if ( m_port != 0 )
	{
		port = std::to_string( m_port );

		uri.portText.first		= port.c_str();
		uri.portText.afterLast	= port.c_str() + port.size();
	}
	
	components = split( m_path, '/' );
	
	if ( components.size() > 0 )
	{
		UriPathSegmentA **path = &uri.pathHead;
		int index = 0;
		
		for ( auto it = components.begin(); it != components.end(); it++ )
		{
			encodedComponents.push_back( encode( *it ) );
			*path = ( UriPathSegmentA* ) malloc( sizeof ( UriPathSegmentA ) );
			( *path )->text.first = encodedComponents[ index ].c_str();
			( *path )->text.afterLast = encodedComponents[ index ].c_str() + encodedComponents[ index ].size();
			( *path )->next = NULL;
			
			path = &( *path )->next;
			
			index++;
		}
	}

	if ( uriToStringCharsRequiredA( &uri, &len ) != URI_SUCCESS )
	{
		goto exit;
	}
	
	len++;

	str = new char[ len ];
	
	if ( !str )
	{
		goto exit;
	}
	
	if ( uriToStringA( str, &uri, len, NULL ) != URI_SUCCESS )
	{
		goto exit;
	}
	
	ret.assign( str );
	
exit:

	uriFreeUriMembersA( &uri );
	
	if ( str )
	{
		delete [] str;
	}
	
	return ret;
}


bool
uri::assign( const std::string& s )
{
	UriParserStateA state;
	UriUriA			uri;
	bool			ok = false;

	state.uri = &uri;
	
	if ( uriParseUriA( &state, s.c_str() ) != URI_SUCCESS )
	{
		goto exit;
	}
	
	m_scheme.assign( uri.scheme.first, uri.scheme.afterLast - uri.scheme.first );
	m_scheme = decode( m_scheme );
	
	m_host.assign( uri.hostText.first, uri.hostText.afterLast - uri.hostText.first );
	m_host = decode( m_host );

	if ( uri.portText.first )
	{
		std::string text;
		
		text.assign( uri.portText.first, uri.portText.afterLast - uri.portText.first );
		m_port = atoi( text.c_str() );
	}
	else if ( ( m_scheme == "http" ) || ( m_scheme == "ws" ) )
	{
		m_port = 80;
	}
	else if ( ( m_scheme == "https" ) || ( m_scheme == "wss" ) )
	{
		m_port = 443;
	}
	
	for ( UriPathSegmentA* path = uri.pathHead; path != NULL; path = path->next )
	{
		std::string temp;
		
		temp.assign( path->text.first, path->text.afterLast - path->text.first );
		temp = decode( temp );
		m_path += "/" + temp;
	}
	
	if ( m_path.size() == 0 )
	{
		m_path = "/";
	}
	
	m_query.assign( uri.query.first, uri.query.afterLast - uri.query.first );
	
	ok = true;
	
exit:

	uriFreeUriMembersA( &uri );
	
	return ok;
}


void
uri::clear()
{
	m_scheme.clear();
	m_host.clear();
	m_port = 0;
	m_path.clear();
	m_query.clear();
}


std::string
uri::encode( const std::string & str )
{
	std::string	ret;
	const char		*last;
	char			*buf = new char[ str.size() * 3 ];
	
	last = uriEscapeA( str.c_str(), buf, false, false );
	ret.assign( buf, last - buf );
	
	return ret;
}


std::string
uri::decode( const std::string & str )
{
	std::string	dummy( str );
	std::string	ret;
	const char		*last;
	
	last = uriUnescapeInPlaceA( ( char* ) dummy.c_str() );
	ret.assign( dummy.c_str(), last );
	
	return ret;
}