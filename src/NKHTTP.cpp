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
 
#include <NetKit/NKHTTP.h>
#include <NetKit/NKSource.h>
#include <NetKit/NKTLS.h>
#include <NetKit/NKBase64.h>
#include <NetKit/cstring.h>
#include <NetKit/NKPlatform.h>
#include <NetKit/NKLog.h>
#include <http_parser.h>
#include <algorithm>
#include <fstream>
#include <regex>
#include <assert.h>
#include <stdarg.h>
#if defined(WIN32)
#	include <shlwapi.h>
#	include <tchar.h>
#	include <errno.h>
#	define strcasecmp _stricmp
#else
#	include <arpa/inet.h>
#	include <netdb.h>
#	include <sys/stat.h>
#	include <sys/errno.h>
#	define TCHAR char
#	define TEXT( X ) X
#endif

#if defined(WIN32)

#	include <winspool.h>
#	include <limits.h>
#	include <sys/types.h>
#	include <string.h>
#	define strcasecmp _stricmp

#endif

using namespace netkit::http;

#if defined( __APPLE__ )
#	pragma mark method implementation
#endif

const std::uint8_t method::delet		= HTTP_DELETE;
const std::uint8_t method::get			= HTTP_GET;
const std::uint8_t method::head			= HTTP_HEAD;
const std::uint8_t method::post			= HTTP_POST;
const std::uint8_t method::put			= HTTP_PUT;
const std::uint8_t method::connect		= HTTP_CONNECT;
const std::uint8_t method::options		= HTTP_OPTIONS;
const std::uint8_t method::trace		= HTTP_TRACE;
const std::uint8_t method::copy			= HTTP_COPY;
const std::uint8_t method::lock			= HTTP_LOCK;
const std::uint8_t method::mkcol		= HTTP_MKCOL;
const std::uint8_t method::move			= HTTP_MOVE;
const std::uint8_t method::propfind		= HTTP_PROPFIND;
const std::uint8_t method::proppatch	= HTTP_PROPPATCH;
const std::uint8_t method::search		= HTTP_SEARCH;
const std::uint8_t method::unlock		= HTTP_UNLOCK;
const std::uint8_t method::report		= HTTP_REPORT;
const std::uint8_t method::mkactivity	= HTTP_MKACTIVITY;
const std::uint8_t method::checkout		= HTTP_CHECKOUT;
const std::uint8_t method::merge		= HTTP_MERGE;
const std::uint8_t method::msearch		= HTTP_MSEARCH;
const std::uint8_t method::notify		= HTTP_NOTIFY;
const std::uint8_t method::subscribe	= HTTP_SUBSCRIBE;
const std::uint8_t method::unsubscribe	= HTTP_UNSUBSCRIBE;
const std::uint8_t method::patch		= HTTP_PATCH;
const std::uint8_t method::purge		= HTTP_PURGE;

std::string
method::to_string( std::uint8_t val )
{
	return http_method_str( ( enum http_method ) val );
}

#if defined( __APPLE__ )
#	pragma mark status implementation
#endif

const std::uint16_t status::error					= -1;
const std::uint16_t status::cont					= 100;
const std::uint16_t status::switching_protocols		= 101;
const std::uint16_t status::ok						= 200;
const std::uint16_t status::created					= 201;
const std::uint16_t status::accepted				= 202;
const std::uint16_t status::not_authoritative		= 203;
const std::uint16_t status::no_content				= 204;
const std::uint16_t status::reset_content			= 205;
const std::uint16_t status::partial_content			= 206;
const std::uint16_t status::multiple_choices		= 300;
const std::uint16_t status::moved_permanently		= 301;
const std::uint16_t status::moved_temporarily		= 302;
const std::uint16_t status::see_other				= 303;
const std::uint16_t status::not_modified			= 304;
const std::uint16_t status::use_proxy				= 305;
const std::uint16_t status::bad_request				= 400;
const std::uint16_t status::unauthorized			= 401;
const std::uint16_t status::payment_required		= 402;
const std::uint16_t status::forbidden				= 403;
const std::uint16_t status::not_found				= 404;
const std::uint16_t status::method_not_allowed		= 405;
const std::uint16_t status::not_acceptable			= 406;
const std::uint16_t status::proxy_authentication	= 407;
const std::uint16_t status::request_timeout			= 408;
const std::uint16_t status::conflict				= 409;
const std::uint16_t status::gone					= 410;
const std::uint16_t status::length_required			= 411;
const std::uint16_t status::precondition			= 412;
const std::uint16_t status::request_too_large		= 413;
const std::uint16_t status::uri_too_long			= 414;
const std::uint16_t status::unsupported_media_type	= 415;
const std::uint16_t status::requested_range			= 416;
const std::uint16_t status::expectation_failed		= 417;
const std::uint16_t status::upgrade_required		= 426;
const std::uint16_t status::server_error			= 500;
const std::uint16_t status::not_implemented			= 501;
const std::uint16_t status::bad_gateway				= 502;
const std::uint16_t status::service_unavailable		= 503;
const std::uint16_t status::gateway_timeout			= 504;
const std::uint16_t status::not_supported			= 505;
const std::uint16_t status::authorized_cancelled	= 1000;
const std::uint16_t status::pki_error				= 1001;
const std::uint16_t status::webif_disabled			= 1002;

std::string
status::to_string( std::uint16_t val )
{
	switch ( val )
	{
		case status::error:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::cont:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::switching_protocols:
		{
			static std::string s( "Switching Protocols" );
			return s;
		}
		break;

		case status::ok:
		{
			static std::string s( "OK" );
			return s;
		}
		break;

		case status::created:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::accepted:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::not_authoritative:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::no_content:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::reset_content:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::partial_content:
		{
			static std::string s( "Error" );
			return s;
		}
		break;


		case status::multiple_choices:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::moved_permanently:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::moved_temporarily:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::see_other:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::not_modified:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::use_proxy:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::bad_request:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::unauthorized:
		{
			static std::string s( "Authorization Required" );
			return s;
		}
		break;

		case status::payment_required:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::forbidden:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::not_found:
		{
			static std::string s( "Not Found" );
			return s;
		}
		break;

		case status::method_not_allowed:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::not_acceptable:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::proxy_authentication:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::request_timeout:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::conflict:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::gone:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::length_required:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::precondition:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::request_too_large:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::uri_too_long:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::unsupported_media_type:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::requested_range:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::expectation_failed:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::upgrade_required:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::server_error:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::not_implemented:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::bad_gateway:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::service_unavailable:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::gateway_timeout:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::not_supported:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::authorized_cancelled:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::pki_error:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::webif_disabled:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		default:
		{
			static std::string s( "Error" );
			return s;
		}
		break;
	}
}

#if defined( __APPLE__ )
#	pragma mark message implementation
#endif

message::message( std::uint16_t major, std::uint16_t minor )
:
	m_major( major ),
	m_minor( minor ),
	m_content_type( "text/plain" ),
	m_content_length( 0 ),
	m_keep_alive( false )
{
}


message::message( const message &that )
:
	m_major( that.m_major ),
	m_minor( that.m_minor ),
	m_content_type( that.m_content_type ),
	m_content_length( that.m_content_length ),
	m_upgrade( that.m_upgrade ),
	m_ws_key( that.m_ws_key ),
	m_keep_alive( that.m_keep_alive )
{
}
	
	
message::~message()
{
}


void
message::add_to_header( const header &heder )
{
	for ( header::const_iterator it = heder.begin(); it != heder.end(); it++ )
	{
		add_to_header( it->first, it->second );
	}
}

	
void
message::add_to_header( const std::string &key, int val )
{
	m_header[ key ] = std::to_string( val );
	
	if ( key == "Content-Length" )
	{
		m_content_length = val;
	}
}


void
message::add_to_header( const std::string &key, const std::string &val )
{
	m_header[ key ] = val;

	if ( key == "Content-Length" )
	{
		m_content_length = atoi( val.c_str() );
	}
	else if ( key == "Content-Type" )
	{
		m_content_type = val;
	}
	else if ( key == "Upgrade" )
	{
		m_upgrade = val;
	}
	else if ( key == "Sec-WebSocket-Key" )
	{
		m_ws_key = val;
	}
}


void
message::remove_from_header( const std::string &key )
{
	m_header.erase( key );
}


void
message::write( const uint8_t *buf, size_t len )
{
	m_ostream.write( reinterpret_cast< const char* >( buf ), len );
}


bool
message::send_body( connection_ref conn ) const
{
	std::string body = m_ostream.str();
	
	if ( body.size() > 0 )
	{
		conn->send( reinterpret_cast< const uint8_t* >( body.c_str() ), body.size(), [=]( int status )
		{
			if ( status != 0 )
			{
			}
		} );
	}
	
	return true;
}

#if defined( __APPLE__ )
#	pragma mark request implementation
#endif

request::request( std::uint16_t major, std::uint16_t minor, int method, const netkit::uri::ref &uri )
:
	message( major, minor ),
	m_method( method ),
	m_uri( uri )
{
	init();
	
	if ( ( major == 1 ) && ( minor == 1 ) )
	{
		m_keep_alive = true;
	}
}

	
request::request( const request &that )
:
	message( that ),
	m_method( that.m_method ),
	m_uri( that.m_uri )
{
	init();
}


request::~request()
{
}


void
request::init()
{
}


void
request::add_to_header( const header& header )
{
	message::add_to_header( header );
}


void
request::add_to_header( const std::string &key, const std::string &val )
{
	message::add_to_header( key, val );
	
	if ( key == "Host" )
    {
		m_host = val;
    }
    else if ( key == "Expect" )
    {
		m_expect = val;
    }
    else if ( key == "Authorization" )
    {
		std::string base64Encoded;
		std::string decoded;
		size_t      pos;

		m_authorization = val;
		
        base64Encoded	= m_authorization.substr( m_authorization.find( ' ' ) + 1 );

        decoded			= codec::base64::decode( base64Encoded );
		pos				= decoded.find( ':' );

		if ( pos != -1 )
		{
			m_username  = decoded.substr( 0, pos );
			m_password  = decoded.substr( pos + 1 );
        }
	}
}


void
request::send_prologue( connection_ref conn ) const
{
	*conn << method::to_string( m_method ) << " " << m_uri->path() << " HTTP/1.1" << http::endl;
}

#if defined( __APPLE__ )
#	pragma mark response implementation
#endif

response::response( std::uint16_t major, std::uint16_t minor, uint16_t status, bool keep_alive )
:
	message( major, minor ),
	m_status( status )
{
	m_keep_alive = keep_alive;
	init();
}


response::response( const response &that )
:
	message( that ),
	m_status( that.m_status )
{
}


response::~response()
{
}


void
response::init()
{
	time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
#if defined( WIN32 )
	localtime_s( &tstruct, &now );
#else
    tstruct = *localtime( &now );
#endif
    strftime(buf, sizeof(buf), "%a, %d %b %Y %I:%M:%S %Z", &tstruct);
	
	add_to_header( "Date", buf );

	if ( m_keep_alive )
	{
		add_to_header( "Connection", "Keep-Alive" );
		add_to_header( "Keep-Alive", "timeout=10" );
	}
}


void
response::send_prologue( connection::ref conn ) const
{
	*conn << "HTTP/" << m_major << "." << m_minor << " " << m_status << " " << status::to_string( m_status ) << endl;
}


#if defined( __APPLE__ )
#	pragma mark connection implementation
#endif


connection::connection()
:
	m_secure( false ),
	m_okay( true )
{
	init();
}


connection::connection( handler::ref h, type t )
:
	m_secure( false ),
	m_okay( true ),
	m_handler( h ),
	m_type( t )
{
	init();
}


connection::~connection()
{
	if ( m_settings )
	{
		delete m_settings;
	}
	
	if ( m_parser )
	{
		delete m_parser;
	}
}


void
connection::init()
{
	m_settings = new http_parser_settings;
	memset( m_settings, 0, sizeof( http_parser_settings ) );
	
	m_parser = new http_parser;
	memset( m_parser, 0, sizeof( http_parser ) );

	m_settings->on_message_begin 	= message_will_begin;
	m_settings->on_url				= uri_was_received;
	m_settings->on_header_field		= header_field_was_received;
	m_settings->on_header_value		= header_value_was_received;
	m_settings->on_body				= body_was_received;
	m_settings->on_headers_complete	= headers_were_received;
	m_settings->on_message_complete	= message_was_received;
	
	http_parser_init( m_parser, HTTP_BOTH );
	m_parser->data = this;
}


void
connection::set_secure( bool val )
{
	if ( !m_secure && val )
	{
		if ( m_type == type::server )
		{
			m_source->add( tls::server::create() );
		}
		else
		{
			m_source->add( tls::client::create() );
		}

		m_secure = true;
	}
}


int
connection::http_major() const
{
	return m_parser->http_major;
}


int
connection::http_minor() const
{
	return m_parser->http_minor;
}


bool
connection::put( message::ref message )
{
	message->send_prologue( this );
	
	for ( message::header::const_iterator it = message->heder().begin(); it != message->heder().end(); it++ )
	{
		*this << it->first << ": " << it->second << http::endl;
	}
			
	*this << http::endl << http::flush;
	
	message->send_body( this );
	
	return true;
}


bool
connection::flush()
{
	std::string msg = m_ostream.str();
		
	if ( msg.size() > 0 )
	{
		nklog( log::verbose, "sending msg: %s", msg.c_str() );
		
		send( reinterpret_cast< const std::uint8_t* >( msg.c_str() ), msg.size(), [=]( int status )
		{
			if ( status != 0 )
			{
			}
		} );
		
		m_ostream.str( "" );
		m_ostream.clear();
	}
		
	return true;
}


bool
connection::process( const std::uint8_t *buf, size_t len )
{
	if ( m_type == type::server )
	{
		server::set_active_connection( this );
	}

	std::streamsize processed	= http_parser_execute( m_parser, m_settings, ( const char* ) buf, len );
	bool			ok			= true;

	if ( processed != len )
	{
		nklog( log::error, "http_parser_execute() failed: bytes read = %ld, processed = %ld", len, processed );
		ok = false;
	}

	if ( m_type == type::server )
	{
		server::set_active_connection( nullptr );
	}
	
	return ok;
}


void
connection::upgrade_to_websocket( sink::ref new_sink )
{
	new_sink->bind( m_source );
	m_source = nullptr;
}


void
connection::upgrade_to_tls()
{
	set_secure( true );
}


int
connection::message_will_begin( http_parser *parser )
{
	connection *self = reinterpret_cast< connection* >( parser->data );
	
	nklog( log::verbose, "reading new message" );

	self->m_parser->upgrade = 0;
	self->m_parse_state	= NONE;
	
	self->m_uri_value.clear();
	self->m_content_type.clear();
	self->m_header_field.clear();
	self->m_header_value.clear();
	self->m_header.clear();
	
	self->m_request = nullptr;
	
	return 0;
}


int
connection::uri_was_received( http_parser *parser, const char *buf, size_t len )
{
	connection	*self = reinterpret_cast< connection* >( parser->data );
	std::string	str( buf, len );
	
	nklog( log::verbose, "uri was received: %s", str.c_str() );
	
	self->m_uri_value.append( str );

	if ( self->m_handler )
	{
		self->m_handler->uri_was_received( parser, buf, len );
	}
	
	return 0;
}


int
connection::header_field_was_received( http_parser *parser, const char *buf, size_t len )
{
	connection	*self = reinterpret_cast< connection* >( parser->data );
	int			ret = 0;
	std::string	str( buf, len );
	
	if ( self->m_parse_state != FIELD )
	{
		if ( ( self->m_header_field.size() > 0 ) && ( self->m_header_value.size() > 0 ) )
		{
			self->m_header[ self->m_header_field ] = self->m_header_value;

			if ( self->m_header_field == "Content-Type" )
			{
				self->m_content_type = self->m_header_value;
			}
		}
		
		self->m_header_field.assign( str );
		self->m_parse_state = FIELD;
	}
	else
	{
		self->m_header_field.append( str );
	}

	if ( self->m_handler )
	{
		self->m_handler->header_field_was_received( parser, buf, len );
	}

	return ret;
}


int
connection::header_value_was_received( http_parser *parser, const char *buf, size_t len )
{
	connection	*self = reinterpret_cast< connection* >( parser->data );
	std::string	str( buf, len );
	
	if ( self->m_parse_state != VALUE )
	{
		self->m_header_value.assign( str );
		self->m_parse_state = VALUE;
	}
	else
	{
		self->m_header_value.append( str );
	}

	if ( self->m_handler )
	{
		self->m_handler->header_value_was_received( parser, buf, len );
	}
	
	return 0;
}

	
int
connection::headers_were_received( http_parser *parser )
{
	connection	*self	= reinterpret_cast< connection* >( parser->data );
	int			ret		= 0;
	
	if ( !self->m_handler )
	{
		if ( !server::resolve( self ) )
		{
			response::ref response = http::response::create( self->http_major(), self->http_minor(), http::status::not_found, false );
			response->add_to_header( "Connection", "Close" );
			response->add_to_header( "Content-Type", "text/html" );
			*response << "<html>Error 404: Content Not Found</html>";
			self->put( response.get() );
			ret = -1;
			goto exit;
		}
	}
	
	/*
	for ( message::header::const_iterator it = self->m_request->heade().begin(); it != self->m_request->heade().end(); it++ )
	{
		nklog( log::verbose, "%s %s\n", it->first.c_str(), it->second.c_str() );
	
		if ( it->first == TEXT( "Content-Length" ) )
		{
			self->m_request->set_content_length( atoi( it->second.c_str() ) );
		}
		else if ( it->first == TEXT( "Content-Type" ) )
		{
			self->m_request->set_content_type( it->secoond );
		}
		else if ( it->first == "Host" )
		{
			self->m_request->set_host( it->second );
		}
		else if ( it->first == "Expect" )
		{
			m_message->set_expect( it->second );
		}
		else if ( *it1 == "Authorization" )
		{
			std::string base64_encoded;
			std::string decoded;
			size_t		pos;
			
			m_authorization	= *it2;
			base64_encoded	= m_authorization.substr( m_authorization.find( ' ' ) + 1 );
			decoded			= codec::base64::decode( base64_encoded );
			pos				= decoded.find( ':' );
			
			if ( pos != -1 )
			{
				m_username	= decoded.substr( 0, pos );
				m_password	= decoded.substr( pos + 1 );
			}
		}
	}
	*/

exit:

	if ( self->m_handler )
	{
		self->m_handler->headers_were_received( parser, self->m_header );
	}

	return ret;
}

	
int
connection::body_was_received( http_parser *parser, const char *buf, size_t len )
{
	connection *self = reinterpret_cast< connection* >( parser->data );

	return self->m_handler->body_was_received( parser, buf, len );
}

	
int
connection::message_was_received( http_parser *parser )
{
	connection *self = reinterpret_cast< connection* >( parser->data );

	return self->m_handler->message_was_received( parser );
}


#if defined( __APPLE__ )
#	pragma mark server implementation
#endif

connection::ref			server::m_active_connection;
connection::list		*server::m_connections;
server::handlers		server::m_handlers;

netkit::sink::ref
server::adopt( netkit::source::ref source, const std::uint8_t *buf, size_t len )
{
	connection::ref sink;
	
	if ( ( len >= 3 ) &&
	     ( ( std::strncasecmp( ( const char* ) buf, "get", 3 ) == 0 ) ||
	       ( std::strncasecmp( ( const char* ) buf, "pos", 3 ) == 0 ) ||
	       ( std::strncasecmp( ( const char* ) buf, "opt", 3 ) == 0 ) ||
	       ( std::strncasecmp( ( const char* ) buf, "hea", 3 ) == 0 ) ) )
	{
		sink = new connection( new server::handler() );

		m_connections->push_back( sink );

		sink->on_close( [=]()
		{
			auto it = std::find_if( m_connections->begin(), m_connections->end(), [=]( connection::ref inserted )
			{
				return ( inserted.get() == sink.get() );
			} );

			assert( it != m_connections->end() );

			m_connections->erase( it );
		} );
	}
	
	return sink.get();
}

	
void
server::bind( std::uint8_t m, const std::string &path, const std::string &type, request_f r )
{
	handler::ref h = new handler( path, type, r );
	
	bind( m, h );
}


void
server::bind( std::uint8_t m, const std::string &path, const std::string &type, request_body_was_received_f rbwr, request_f r )
{
	handler::ref h = new handler( path, type, rbwr, r );
	
	bind( m, h );
}


void
server::bind( std::uint8_t m, const std::string &path, const std::string &type, request_will_begin_f rwb, request_body_was_received_f rbwr, request_f r )
{
	handler::ref h = new handler( path, type, rwb, rbwr, r );
	
	bind( m, h );
}


void
server::bind( std::uint8_t m, const std::string &path, netkit::sink::ref s )
{
	handler::ref h = new handler( path, s );
	
	bind( m, h );
}


void
server::bind( std::uint8_t m, handler::ref h )
{
	handlers::iterator it = m_handlers.find( m );
	
	if ( it != m_handlers.end() )
	{
		it->second.push_back( h );
	}
	else
	{
		handler::list l;
		
		l.push_back( h );
		
		m_handlers[ m ] = l;
	}
}


bool
server::resolve( connection::ref conn )
{
	server::handler::ref handler;
	bool resolved = false;
	
	conn->m_method = conn->m_parser->method;
	
	auto it = m_handlers.find( conn->m_parser->method );
	
	if ( it != m_handlers.end() )
	{
		for ( handler::list::iterator it2 = it->second.begin(); it2 != it->second.end(); it2++ )
		{
			try
			{
				std::regex regex1( regexify( ( *it2 )->m_path ) );
				std::regex regex2( regexify( ( *it2 )->m_type ) );

				if ( std::regex_search( conn->m_uri_value, regex1 ) && std::regex_search( conn->m_content_type, regex2 ) )
				{
					conn->m_handler =   ( *it2 ).get();
					handler = *it2;
					break;
				}
			}
			catch ( std::exception &exc )
			{
				nklog( log::error, "caught exception in regex: %s", exc.what() );
			}
		}
	}
	
	if ( !handler )
	{
		nklog( log::error, "unable to find binding for method %d -> %s", conn->m_method, conn->m_uri_value.c_str() );
		goto exit;
	}
	
	if ( handler->m_rwb )
	{
		conn->m_request = handler->m_rwb( conn->http_major(), conn->http_minor(), conn->m_parser->method, new uri( conn->m_uri_value ) );
	}
	else
	{
		conn->m_request = http::request::create( conn->http_major(), conn->http_minor(), conn->m_parser->method, new uri( conn->m_uri_value ) );
	}

	if ( !conn->m_request )
	{
		nklog( log::error, "unable to create request for method %d -> %s", conn->m_method, conn->m_uri_value.c_str() );
		goto exit;
	}
	
	conn->m_request->add_to_header( conn->m_header );
	
	if ( conn->m_request->expect() == "100-continue" )
	{
		response::ref response = http::response::create( conn->http_major(), conn->http_minor(), http::status::cont, false );
		conn->put( response.get() );
	}
	
	resolved = true;

exit:

	return resolved;
}


std::string
server::regexify( const std::string &in )
{
	std::string out( in );

	replace( out, "\\", "\\\\" );
    replace( out, "^", "\\^");
	replace( out, ".", "\\.");
	replace( out, "$", "\\$");
	replace( out, "|", "\\|");
    replace( out, "(", "\\(");
    replace( out, ")", "\\)");
    replace( out, "[", "\\[");
    replace( out, "]", "\\]");
    replace( out, "*", "\\*");
    replace( out, "+", "\\+");
    replace( out, "?", "\\?");
    replace( out, "/", "\\/");
	replace( out, "\\?", ".");
    replace( out, "\\*", ".*");

	return out;
}


void
server::replace( std::string& str, const std::string& oldStr, const std::string& newStr)
{
	size_t pos = 0;

	while ( ( pos = str.find( oldStr, pos ) ) != std::string::npos )
	{
		str.replace(pos, oldStr.length(), newStr);
		pos += newStr.length();
	}
}


int
server::handler::uri_was_received( http_parser *parser, const char *buf, size_t len )
{
	return 0;
}


int
server::handler::header_field_was_received( http_parser *parser, const char *buf, size_t len )
{
	return 0;
}


int
server::handler::header_value_was_received( http_parser *parser, const char *buf, size_t len )
{
	return 0;
}


int
server::handler::headers_were_received( http_parser *parser, message::header &header )
{
	return 0;
}


int
server::handler::body_was_received( http_parser *parser, const char *buf, size_t len )
{
	connection *conn = reinterpret_cast< connection* >( parser->data );

	return m_rbwr( conn->m_request, ( const uint8_t* ) buf, len, [=]( response::ref response, bool close )
	{
		conn->put( response.get() );
		
		if ( close )
		{
			conn->close();
		}
	} );
}


int
server::handler::message_was_received( http_parser *parser )
{
	connection *conn = reinterpret_cast< connection* >( parser->data );

	return m_r( conn->m_request, [=]( response::ref response, bool close )
	{
		conn->put( response.get() );

		if ( close )
		{
			conn->close();
		}
	} );
}


#if defined( __APPLE__ )
#	pragma mark client implementation
#endif

client::client( const request::ref &request, auth_f auth_func, response_f response_func, uri_recv_f uri_func, header_field_recv_f header_field_func, header_value_recv_f header_value_func, headers_recv_f headers_func )
:
	m_request( request )
{
	m_handler = new handler( response_func, uri_func, header_field_func, header_value_func, headers_func );
	m_connection = new connection( m_handler.get(), connection::type::client );
}


client::~client()
{
}


void
client::send( const request::ref &request, response_f response_func )
{
	client *self = new client( request, [=]( request::ref &request, uint32_t status )
	{
		return false;
	}, response_func, [=]( std::string& )
	{
	}, [=]( std::string& )
	{
	}, [=] ( std::string& )
	{
	}, [=] ( message::header& )
	{
	});

	self->send_request();
}


void
client::send( const request::ref &request, response_f response_func, uri_recv_f uri_func, header_field_recv_f header_field_func, header_value_recv_f header_value_func, headers_recv_f headers_func )
{
	client *self = new client( request, [=]( request::ref &request, uint32_t status )
	{
		return false;
	}, response_func, uri_func, header_field_func, header_value_func, headers_func );

	self->send_request();
}


void
client::send_request()
{
	m_connection->connect( m_request->uri(), [=]( int status, const endpoint::ref &peer )
	{
		if ( status == 0 )
		{
			if ( m_request->uri()->scheme() == "https" )
			{
				m_connection->set_secure( true );
			}

			m_request->add_to_header( "Host", m_request->uri()->host() );
			m_request->add_to_header( "User-Agent", "NetKit/2 " + platform::machine_description() );

			if ( m_request->body().length() > 0 )
			{
				m_request->add_to_header( "Content-Length", m_request->body().length() );
			}

			m_connection->put( m_request.get() );
		}
	} );
}


int
client::handler::uri_was_received( http_parser *parser, const char *buf, size_t len )
{
	std::string uri( buf, len );
	m_uri_recv_func( uri );

	return 0;
}


int
client::handler::header_field_was_received( http_parser *parser, const char *buf, size_t len )
{
	std::string field( buf, len );
	m_header_field_recv_func( field );

	return 0;
}


int
client::handler::header_value_was_received( http_parser *parser, const char *buf, size_t len )
{
	std::string value( buf, len );
	m_header_value_recv_func( value );

	return 0;
}


int
client::handler::headers_were_received( http_parser *parser, message::header &header )
{
	m_headers_recv_func( header );

	m_response = response::create( parser->http_major, parser->http_minor, parser->status_code, false );
	m_response->add_to_header( header );

	return 0;
}


int
client::handler::body_was_received( http_parser *parser, const char *buf, size_t len )
{
	m_response->write( reinterpret_cast< const uint8_t* >( buf ), len );

	return 0;
}


int
client::handler::message_was_received( http_parser *parser )
{
	m_response_func( m_response );

	return 0;
}