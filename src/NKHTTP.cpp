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
#include <NetKit/NKWebSocket.h>
#include <NetKit/NKTLS.h>
#include <NetKit/NKBase64.h>
#include <NetKit/cstring.h>
#include <NetKit/NKPlatform.h>
#include <NetKit/NKProxy.h>
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

/*
case 100: $string = 'Continue'; break;
		case 101: $string = 'Switching Protocols'; break;
		case 102: $string = 'Processing'; break; // WebDAV
		case 122: $string = 'Request-URI too long'; break; // Microsoft

		// 2xx Success
		case 200: $string = 'OK'; break;
		case 201: $string = 'Created'; break;
		case 202: $string = 'Accepted'; break;
		case 203: $string = 'Non-Authoritative Information'; break; // HTTP/1.1
		case 204: $string = 'No Content'; break;
		case 205: $string = 'Reset Content'; break;
		case 206: $string = 'Partial Content'; break;
		case 207: $string = 'Multi-Status'; break; // WebDAV

		// 3xx Redirection
		case 300: $string = 'Multiple Choices'; break;
		case 301: $string = 'Moved Permanently'; break;
		case 302: $string = 'Found'; break;
		case 303: $string = 'See Other'; break; //HTTP/1.1
		case 304: $string = 'Not Modified'; break;
		case 305: $string = 'Use Proxy'; break; // HTTP/1.1
		case 306: $string = 'Switch Proxy'; break; // Depreciated
		case 307: $string = 'Temporary Redirect'; break; // HTTP/1.1

		// 4xx Client Error
		case 400: $string = 'Bad Request'; break;
		case 401: $string = 'Unauthorized'; break;
		case 402: $string = 'Payment Required'; break;
		case 403: $string = 'Forbidden'; break;
		case 404: $string = 'Not Found'; break;
		case 405: $string = 'Method Not Allowed'; break;
		case 406: $string = 'Not Acceptable'; break;
		case 407: $string = 'Proxy Authentication Required'; break;
		case 408: $string = 'Request Timeout'; break;
		case 409: $string = 'Conflict'; break;
		case 410: $string = 'Gone'; break;
		case 411: $string = 'Length Required'; break;
		case 412: $string = 'Precondition Failed'; break;
		case 413: $string = 'Request Entity Too Large'; break;
		case 414: $string = 'Request-URI Too Long'; break;
		case 415: $string = 'Unsupported Media Type'; break;
		case 416: $string = 'Requested Range Not Satisfiable'; break;
		case 417: $string = 'Expectation Failed'; break;
		case 422: $string = 'Unprocessable Entity'; break; // WebDAV
		case 423: $string = 'Locked'; break; // WebDAV
		case 424: $string = 'Failed Dependency'; break; // WebDAV
		case 425: $string = 'Unordered Collection'; break; // WebDAV
		case 426: $string = 'Upgrade Required'; break;
		case 449: $string = 'Retry With'; break; // Microsoft
		case 450: $string = 'Blocked'; break; // Microsoft

		// 5xx Server Error
		case 500: $string = 'Internal Server Error'; break;
		case 501: $string = 'Not Implemented'; break;
		case 502: $string = 'Bad Gateway'; break;
		case 503: $string = 'Service Unavailable'; break;
		case 504: $string = 'Gateway Timeout'; break;
		case 505: $string = 'HTTP Version Not Supported'; break;
		case 506: $string = 'Variant Also Negotiates'; break;
		case 507: $string = 'Insufficient Storage'; break; // WebDAV
		case 509: $string = 'Bandwidth Limit Exceeded'; break; // Apache
		case 510: $string = 'Not Extended'; break;
*/
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
			static std::string s( "Continue" );
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
			static std::string s( "Created" );
			return s;
		}
		break;

		case status::accepted:
		{
			static std::string s( "Accepted" );
			return s;
		}
		break;

		case status::not_authoritative:
		{
			static std::string s( "Non-Authoritative Information" );
			return s;
		}
		break;

		case status::no_content:
		{
			static std::string s( "No Content" );
			return s;
		}
		break;

		case status::reset_content:
		{
			static std::string s( "Reset Content" );
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
message::add_to_header( const header &heeder )
{
	for ( auto it = heeder.begin(); it != heeder.end(); it++ )
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


std::string
message::find_in_header( const std::string &key ) const
{
	std::string val;
	auto		it = m_header.find( key );

	if ( it != m_header.end() )
	{
		val = it->second;
	}

	return val;
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

request::request( int method, std::uint16_t major, std::uint16_t minor, const netkit::uri::ref &uri )
:
	message( major, minor ),
	m_max_redirects( 3 ),
	m_num_redirects( 0 ),
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
	nklog( log::verbose, "" );
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
	*conn << method::to_string( m_method ) << " ";
	
	if ( proxy::get()->is_null() || !proxy::get()->is_http() || ( m_uri->scheme() == "https" ) )
	{
		*conn << m_uri->path();
	}
	else
	{
		*conn << m_uri->to_string();
	}

	if ( !m_uri->query().empty() )
	{
		*conn << "?" << m_uri->query();
	}

	*conn << " HTTP/1.1" << http::endl;
}


void
request::auth( int status )
{
	if ( m_auth )
	{
		m_auth( status );
	}
}


void
request::headers_reply( response_ref response )
{
	if ( m_headers_reply )
	{
		m_headers_reply( response );
	}
}


void
request::body_reply( response_ref response, const std::uint8_t *buf, std::size_t len )
{
	if ( m_body_reply )
	{
		m_body_reply( response, buf, len );
	}
	else
	{
		response->write( buf, len );
	}
}


void
request::reply( response_ref response )
{
	if ( m_reply )
	{
		m_reply( response );
	}
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
	nklog( log::verbose, "" );
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


connection::connection( handler::ref h )
:
	m_secure( false ),
	m_okay( true ),
	m_handler( h )
{
	init();
}


connection::~connection()
{
	nklog( log::verbose, "" );

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
connection::set_secure( bool val, bool is_server )
{
	if ( !m_secure && val )
	{
		if ( is_server )
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
connection::method() const
{
	return m_parser->method;
}


int
connection::status_code() const
{
	return m_parser->status_code;
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
	m_ostream.clear();
	
	message->send_prologue( this );
	
	for ( auto it = message->heeder().begin(); it != message->heeder().end(); it++ )
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
	m_handler->process_will_begin( this );

	std::streamsize processed	= http_parser_execute( m_parser, m_settings, ( const char* ) buf, len );
	bool			ok			= true;

	if ( processed != len )
	{
		nklog( log::error, "http_parser_execute() failed: bytes read = %ld, processed = %ld", len, processed );
		ok = false;
	}

	m_handler->process_did_end( this );

	return ok;
}


bool
connection::upgrade_to_ws( const request::ref &request, server::response_f reply )
{
	http::response::ref response;

	nklog( log::verbose, "upgrading http connection to websocket" );

	response = new http::response( 1, 1, http::status::switching_protocols, false );
	response->add_to_header( "Upgrade", "websocket" );
	response->add_to_header( "Connection", "Upgrade" );
	response->add_to_header( "Sec-WebSocket-Accept", ws::server::accept_key( request->ws_key() ) );

	reply( response, false );

	m_source->add( ws::server::create() );

	return true;
}


bool
connection::upgrade_to_tls( const request::ref &request, server::response_f reply )
{
	http::response::ref response;

	nklog( log::verbose, "upgrading http connection to tls" );

	response = new http::response( 1, 1, http::status::switching_protocols, false );
	response->add_to_header( "Connection", "Upgrade" );
	response->add_to_header( "Upgrade", "TLS/1.0, HTTP/1.1" );

	reply( response, false );

	m_source->add( tls::server::create() );

	return true;
}


void
connection::upgrade( sink::ref sink )
{
	for ( auto it = attrs_begin(); it != attrs_end(); it++ )
	{
		sink->set_value_for_key( it->first, it->second );
	}

	sink->bind( m_source );

	unbind();
}


int
connection::message_will_begin( http_parser *parser )
{
	connection *self = reinterpret_cast< connection* >( parser->data );
	assert( self );
	assert( self->m_handler );
	
	nklog( log::verbose, "reading new message" );

	self->m_parser->upgrade = 0;
	self->m_parse_state	= NONE;
	
	self->m_header_field.clear();
	self->m_header_value.clear();
	self->m_header.clear();

	self->m_handler->message_will_begin( self );
	
	return 0;
}


int
connection::uri_was_received( http_parser *parser, const char *buf, size_t len )
{
	connection *self = reinterpret_cast< connection* >( parser->data );
	assert( self );
	assert( self->m_handler );

	return self->m_handler->uri_was_received( self, buf, len );
}


int
connection::header_field_was_received( http_parser *parser, const char *buf, size_t len )
{
	connection	*self = reinterpret_cast< connection* >( parser->data );
	assert( self );
	assert( self->m_handler );

	if ( self->m_parse_state != FIELD )
	{
		if ( ( self->m_header_field.size() > 0 ) && ( self->m_header_value.size() > 0 ) )
		{
			self->m_header[ self->m_header_field ] = self->m_header_value;
		}
		
		self->m_header_field.assign( buf, buf + len );
		self->m_parse_state = FIELD;
	}
	else
	{
		self->m_header_field.append( buf, buf + len );
	}

	return 0;
}


int
connection::header_value_was_received( http_parser *parser, const char *buf, size_t len )
{
	connection *self = reinterpret_cast< connection* >( parser->data );
	assert( self );
	assert( self->m_handler );

	if ( self->m_parse_state != VALUE )
	{
		self->m_header_value.assign( buf, buf + len );
		self->m_parse_state = VALUE;
	}
	else
	{
		self->m_header_value.append( buf, buf + len );
	}

	return 0;
}

	
int
connection::headers_were_received( http_parser *parser )
{
	connection *self = reinterpret_cast< connection* >( parser->data );
	assert( self );
	assert( self->m_handler );

	if ( ( self->m_header_field.size() > 0 ) && ( self->m_header_value.size() > 0 ) )
	{
		self->m_header[ self->m_header_field ] = self->m_header_value;
	}

	return self->m_handler->headers_were_received( self, self->m_header );
}

	
int
connection::body_was_received( http_parser *parser, const char *buf, size_t len )
{
	connection *self = reinterpret_cast< connection* >( parser->data );
	assert( self );
	assert( self->m_handler );

	return self->m_handler->body_was_received( self, buf, len );
}

	
int
connection::message_was_received( http_parser *parser )
{
	connection *self = reinterpret_cast< connection* >( parser->data );
	assert( self );
	assert( self->m_handler );

	return self->m_handler->message_was_received( self );
}


#if defined( __APPLE__ )
#	pragma mark server implementation
#endif

connection::ref			server::m_active_connection;
connection::list		*server::m_connections;
server::bindings		server::m_bindings;

netkit::sink::ref
server::adopt( netkit::source::ref source )
{
	connection::ref sink;
	
	sink = new connection( new server::handler );

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
	
	return sink.get();
}

	
void
server::bind( std::uint8_t m, const std::string &path, const std::string &type, request_f r )
{
	binding::ref b = new binding( path, type, r );
	
	bind( m, b );
}


void
server::bind( std::uint8_t m, const std::string &path, const std::string &type, request_body_was_received_f rbwr, request_f r )
{
	binding::ref b = new binding( path, type, rbwr, r );
	
	bind( m, b );
}


void
server::bind( std::uint8_t m, const std::string &path, const std::string &type, request_will_begin_f rwb, request_body_was_received_f rbwr, request_f r )
{
	binding::ref b = new binding( path, type, rwb, rbwr, r );
	
	bind( m, b );
}


void
server::bind( std::uint8_t m, const std::string &path, netkit::sink::ref s )
{
//	binding::ref b = new binding( path, s );
	
//	bind( m, b );
}


void
server::bind( std::uint8_t m, binding::ref b )
{
	bindings::iterator it = m_bindings.find( m );
	
	if ( it != m_bindings.end() )
	{
		it->second.push_back( b );
	}
	else
	{
		binding::list l;
		
		l.push_back( b );
		
		m_bindings[ m ] = l;
	}
}


server::binding::ref
server::resolve( connection::ref conn, const message::header &header )
{
	server::binding::ref	binding;
//	handler::ref	handler = dynamic_pointer_cast< handler::ref >( conn->handler() );
	handler::ref			handler = dynamic_cast< server::handler* >( conn->handler().get() );
	
	auto it1 = m_bindings.find( conn->m_parser->method );
	
	if ( it1 != m_bindings.end() )
	{
		std::string content_type;
		auto it2 = header.find( "Content-Type" );

		if ( it2 != header.end() )
		{
			content_type = it2->second;
		}

		for ( auto it3 = it1->second.begin(); it3 != it1->second.end(); it3++ )
		{
			try
			{
				std::regex	regex1( regexify( ( *it3 )->m_path ) );
				std::regex	regex2( regexify( ( *it3 )->m_type ) );

				if ( ( ( *it3 )->m_type == "*" && std::regex_search( handler->m_uri, regex1 ) ) ||
					( std::regex_search( handler->m_uri, regex1 ) && std::regex_search( content_type, regex2 ) ) )
				{
					binding = ( *it3 ).get();
					break;
				}
			}
			catch ( std::exception &exc )
			{
				nklog( log::error, "caught exception in regex: %s", exc.what() );
			}
		}
	}
	
	if ( !binding )
	{
		nklog( log::error, "unable to find binding for method %d -> %s", conn->method(), handler->m_uri.c_str() );
		goto exit;
	}
	
exit:

	return binding;
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

#if defined( __APPLE__ )
#	pragma mark server::handler implementation
#endif

server::handler::handler()
{
}


server::handler::~handler()
{
	nklog( log::verbose, "" );
}


void
server::handler::process_will_begin( connection::ref connection )
{
	server::set_active_connection( connection.get() );
}


void
server::handler::message_will_begin( connection::ref connection )
{
	m_binding = nullptr;
	m_uri.clear();
}


int
server::handler::uri_was_received( connection::ref connection, const char *buf, size_t len )
{
	nklog( log::verbose, "uri was received" );
	
	m_uri.append( buf, buf + len );

	return 0;
}


int
server::handler::headers_were_received( connection::ref connection, message::header &header )
{
	int ret = -1;

	m_binding = server::resolve( connection, header );
	
	if ( !m_binding )
	{
		response::ref response = new http::response( connection->http_major(), connection->http_minor(), http::status::not_found, false );
		response->add_to_header( "Connection", "Close" );
		response->add_to_header( "Content-Type", "text/html" );
		*response << "<html>Error 404: Content Not Found</html>";
		response->add_to_header( "Content-Length", response->body().size() );
		connection->put( response.get() );
		goto exit;
	}

	m_request = m_binding->m_rwb( connection->method(), connection->http_major(), connection->http_minor(), new uri( m_uri ) );

	if ( !m_request )
	{
		response::ref response = new http::response( connection->http_major(), connection->http_minor(), http::status::not_found, false );
		response->add_to_header( "Connection", "Close" );
		response->add_to_header( "Content-Type", "text/html" );
		*response << "<html>Error 404: Content Not Found</html>";
		connection->put( response.get() );
		goto exit;
	}

	m_request->add_to_header( header );
	
	if ( m_request->expect() == "100-continue" )
	{
		response::ref response = new http::response( connection->http_major(), connection->http_minor(), http::status::cont, false );
		connection->put( response.get() );
	}

	ret = 0;

exit:

	return ret;
}


int
server::handler::body_was_received( connection::ref connection, const char *buf, size_t len )
{
	assert( m_binding );
	assert( m_request );

	return m_binding->m_rbwr( m_request, ( const uint8_t* ) buf, len, [=]( response::ref response, bool close ) mutable
	{
		connection->put( response.get() );
		
		if ( close )
		{
			connection->close();
		}

		m_request = nullptr;
	} );
}


int
server::handler::message_was_received( connection::ref connection )
{
	assert( m_binding );
	assert( m_request );

	return m_binding->m_r( m_request, [=]( response::ref response, bool close ) mutable
	{
		connection->put( response.get() );

		if ( close )
		{
			connection->close();
		}

		m_request = nullptr;
	} );
}


void
server::handler::process_did_end( connection::ref connection )
{
	server::set_active_connection( nullptr );
}


#if defined( __APPLE__ )
#	pragma mark client implementation
#endif

client::client( const request::ref &request )
:
	m_connection( new connection( this ) ),
	m_request( request ),
	m_done( false )
{
}


client::~client()
{
	nklog( log::verbose, "" );
}


void
client::send( const request::ref &request )
{
	client *self = new client( request );
	
	self->really_send();
}


void
client::really_send()
{
	m_connection->connect( m_request->uri(), [=]( int status, const endpoint::ref &peer )
	{
		if ( status == 0 )
		{
			m_request->add_to_header( "Host", m_request->uri()->host() );
			m_request->add_to_header( "User-Agent", "NetKit/2 " + platform::machine_description() );
			m_request->add_to_header( "Connection", "keep-alive" );

			if ( proxy::get()->is_http() && ( proxy::get()->authorization().size() > 0 ) )
			{
				m_request->add_to_header( "Proxy-Authorization", "basic " + proxy::get()->authorization() );
				m_request->add_to_header( "Proxy-Connection", "keep-alive" );
			}

			if ( m_request->body().length() > 0 )
			{
				m_request->add_to_header( "Content-Length", m_request->body().length() );
			}

			m_connection->put( m_request.get() );
		}
		else
		{
			nklog( log::error, "received error %d trying to connect to uri '%s'", status, m_request->uri()->to_string().c_str() );
			m_response = new response( m_connection->http_major(), m_connection->http_minor(), 0, false );
			m_request->reply( m_response );
		}
	} );
}


void
client::process_will_begin( connection::ref connection )
{
}


void
client::message_will_begin( connection::ref connection )
{
}


int
client::uri_was_received( connection::ref connection, const char *buf, size_t len )
{
	assert( 0 );
	return 0;
}


int
client::headers_were_received( connection::ref connection, message::header &header )
{
	int ret = 0;

	m_response = new response( connection->http_major(), connection->http_minor(), connection->status_code(), false );
	m_response->add_to_header( header );

	if ( ( connection->status_code() == http::status::moved_permanently ) ||
	     ( connection->status_code() == http::status::moved_temporarily ) )
	{
		for ( auto it = header.begin(); it != header.end(); it++ )
		{
			if ( ( it->first == "Location" ) && m_request->can_redirect() )
			{
				m_redirect = it->second;
				m_request->redirect();
			}
		}
	}
	
	if ( m_redirect.size() > 0 )
	{
		nklog( log::verbose, "redirecting HTTP request to %s", m_redirect.c_str() );
		
		m_request->set_uri( new netkit::uri( m_redirect ) );

		client::send( m_request );
	}
	else if ( connection->status_code() != http::status::proxy_authentication )
	{
		m_request->headers_reply( m_response );
		m_request->on_headers_reply( nullptr );
	}

	return ret;
}


int
client::body_was_received( connection::ref connection, const char *buf, size_t len )
{
	if ( ( m_redirect.size() == 0 ) && ( connection->status_code() != http::status::proxy_authentication ) )
	{
		m_request->body_reply( m_response, ( std::uint8_t* ) buf, len );
	}
	
	return 0;
}


int
client::message_was_received( connection::ref connection )
{
	if ( ( connection->status_code() == http::status::proxy_authentication ) && proxy::auth_challenge() )
	{
		m_request->add_to_header( "Proxy-Authorization", "basic " + proxy::get()->authorization() );
		m_request->add_to_header( "Proxy-Connection", "keep-alive" );
		
		client::send( m_request );
	}
	else if ( m_redirect.size() == 0 )
	{
		m_request->reply( m_response );
		m_request->on_auth( nullptr );
		m_request->on_body_reply( nullptr );
		m_request->on_reply( nullptr );
	}

	m_done = true;

	return 0;
}


void
client::process_did_end( connection::ref connection )
{
	if ( m_done )
	{
		m_connection->close();
		m_connection = nullptr;
	}
}
