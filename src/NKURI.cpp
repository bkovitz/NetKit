#include "NKURI.h"
#include "cstring.h"
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
uri::recompose() const
{
	std::string	ret;
	UriUriStructA	uri;
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
		char buf[ 12 ];
		
		sprintf( buf, "%d", m_port );
		uri.portText.first		= buf;
		uri.portText.afterLast	= buf + strlen( buf );
	}
	
	components = split( m_path, '/' );
	
	if ( components.size() > 0 )
	{
		UriPathSegmentA **path = &uri.pathHead;
		int index = 0;
		
		for ( std::vector< std::string>::iterator it = components.begin(); it != components.end(); it++ )
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


void
uri::assign( const std::string& s )
{
	UriParserStateA state;
	UriUriA			uri;

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
	else if ( m_scheme == "http" )
	{
		m_port = 80;
	}
	else if ( m_scheme == "https" )
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
	
	m_query.assign( uri.query.first, uri.query.afterLast - uri.query.first );
	
exit:

	uriFreeUriMembersA( &uri );
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