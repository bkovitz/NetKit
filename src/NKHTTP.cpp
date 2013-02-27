#include "NKHTTP.h"
#include "NKSocket.h"
#include "NKBase64.h"
#include "cstring.h"
#include "NKPlatform.h"
#include "NKLog.h"
#include <http_parser.h>
#include <fstream>
#include <regex>
#include <assert.h>
#include <stdarg.h>
#include <openssl/ssl.h>
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
method::as_string( std::uint8_t val )
{
	return http_method_str( ( enum http_method ) val );
}


#if defined( __APPLE__ )
#	pragma mark message implementation
#endif

message::message()
:
	m_content_type( "text/plain" ),
	m_content_length( 0 )
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
	char buf[ 1024 ];
		
	std::sprintf_s( buf, sizeof( buf ), sizeof( buf ), "%d", val );
	m_header.push_back( make_pair( key, buf ) );
}


void
message::add_to_header( const std::string &key, const std::string &val )
{
	m_header.push_back( make_pair( key, val ) );
	
	if ( key == TEXT( "Content-Length" ) )
	{
		m_content_length = atoi( val.c_str() );
	}
	else if ( key == TEXT( "Content-Type" ) )
	{
		m_content_type = val;
	}
}


void
message::remove_from_header( const std::string &key )
{
	for ( auto it = m_header.begin(); it != m_header.end(); it++ )
	{
		if ( it->first == key )
		{
			m_header.erase( it );
			break;
		}
	}
}


void
message::write( const uint8_t *buf, size_t len )
{
	m_ostream.write( reinterpret_cast< const char* >( buf ), len );
}


bool
message::send_body( connection_ptr conn ) const
{
	std::string body = m_ostream.str();
	
	conn->send( reinterpret_cast< const uint8_t* >( body.c_str() ), body.size() );
	
	return true;
}

#if defined( __APPLE__ )
#	pragma mark request implementation
#endif

request::request( int method, const uri::ptr &uri )
:
	m_method( method ),
	m_uri( uri )
{
	init();
}

	
request::request( const request &r )
:
	m_method( r.m_method ),
	m_uri( r.m_uri )
{
	init();
}


request::~request()
{
}


void
request::init()
{
	add_to_header( "Host", m_uri->host() );
	add_to_header( "Connection", "keep-alive" );
}


void
request::send_prologue( connection_ptr conn ) const
{
	*conn << m_method << " " << m_uri->path() << " HTTP/1.1" << http::endl;
}

#if defined( __APPLE__ )
#	pragma mark response implementation
#endif

response::response()
:
	m_status( 200 )
{
	init();
}


response::response( int status )
:
	m_status( status )
{
	init();
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
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%a, %d %b %Y %I:%M:%S %Z", &tstruct);
	
	add_to_header( "Date", buf );
}


void
response::send_prologue( connection::ptr conn ) const
{
	*conn << "HTTP/" << conn->http_major() << "." << conn->http_minor() << " " << m_status << " OK\r\n";
}


#if defined( __APPLE__ )
#	pragma mark connection implementation
#endif

connection::handlers connection::m_handlers;

connection::connection( const source::ptr &source )
:
	sink( source ),
	m_okay( true )
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
	
	http_parser_init( m_parser, HTTP_REQUEST );
	m_parser->data = this;
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
connection::bind( std::uint8_t m, const std::string &path, const std::string &type, request_f r )
{
	handler::ptr h = new handler( path, type, r );
	
	bind( m, h );
}


void
connection::bind( std::uint8_t m, const std::string &path, const std::string &type, request_body_was_received_f rbwr, request_f r )
{
	handler::ptr h = new handler( path, type, rbwr, r );
	
	bind( m, h );
}


void
connection::bind( std::uint8_t m, const std::string &path, const std::string &type, request_will_begin_f rwb, request_body_was_received_f rbwr, request_f r )
{
	handler::ptr h = new handler( path, type, rwb, rbwr, r );
	
	bind( m, h );
}


void
connection::bind( std::uint8_t m, handler::ptr h )
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


netkit::sink::ptr
connection::adopt( source::ptr source, const std::uint8_t *buf, size_t len )
{
	sink::ptr conn;
	
	if ( ( len >= 3 ) &&
	     ( ( std::strncasecmp( ( const char* ) buf, "get", 3 ) == 0 ) ||
	       ( std::strncasecmp( ( const char* ) buf, "pos", 3 ) == 0 ) ||
	       ( std::strncasecmp( ( const char* ) buf, "opt", 3 ) == 0 ) ||
	       ( std::strncasecmp( ( const char* ) buf, "hea", 3 ) == 0 ) ) )
	{
		conn = new connection( source );
	}
	
	return conn;
}

#if 0
	
		if ( conn )
		{
				conn->set_request_will_begin_handler( [ this, conn ]( const netkit::uri::ptr &uri ) -> request::ptr
				{
					connection::ptr temp( conn );
					
					for ( handlers::iterator it = m_handlers.begin(); it != m_handlers.end(); it++ )
					{
						if ( uri->path().find( it->first ) == 0 )
						{
							return it->second->message_will_begin( uri );
						}
					}
					
					response::ptr response = new http::response( 404 );
					response->add_to_header( "Connection", "Close" );
					response->add_to_header( "Content-Type", "text/html" );
					*response << "<html>Error 404: Content Not Found</html>";
					temp->send( response );
					
					return request::ptr();
				} );
				
				conn->set_headers_were_received_handler( [ this, conn ]( message::ptr message ) -> int
				{
					request::ptr	request = dynamic_pointer_cast< http::request >( message );
					handler			*hdlr	= reinterpret_cast< handler* >( request->context() );
					int				ret;
					
					if ( hdlr )
				 	{
						ret = hdlr->headers_were_received( conn, request );
					}
					else
					{
						connection::ptr temp( conn );
						response::ptr response = new http::response( 500 );
						response->add_to_header( "Connection", "Close" );
						response->add_to_header( "Content-Type", "text/html" );
						*response << "<html>Error 500: Internal Server Error</html>";
						temp->send( response );
						ret = -1;
					}
					
					return ret;
				} );
				
				conn->set_message_was_received_handler( [ this, conn ]( message::ptr message ) -> int
				{
					request::ptr	request = dynamic_pointer_cast< http::request >( message );
					handler			*hdlr	= reinterpret_cast< handler* >( request->context() );
					int				ret;
					
					if ( hdlr )
				 	{
						ret = hdlr->message_was_received( conn, request );
					}
					else
					{
						connection::ptr temp( conn );
						response::ptr response = new http::response( 500 );
						response->add_to_header( "Connection", "Close" );
						response->add_to_header( "Content-Type", "text/html" );
						*response << "<html>Error 500: Internal Server Error</html>";
						temp->send( response );
						ret = -1;
					}
					
					return ret;
				} );
				
				conn->set_disconnect_handler( [&]()
				{
					m_connections.remove( conn );
				} );
				
				m_connections.push_back( conn );
			}
		}
	}
	
	return ( conn ) ? true : false;
}

#endif


bool
connection::put( message::ptr message )
{
	auto len = message->body().size();
	
	message->send_prologue( this );
	
	if ( len > 0 )
	{
		message->add_to_header( "Content-Length", ( int ) len );
	}
	else
	{
		message->remove_from_header( "Content-Length" );
	}

	for ( message::header::const_iterator it = message->heder().begin(); it != message->heder().end(); it++ )
	{
		*this << it->first << ": " << it->second << http::endl;
	}
			
	*this << http::endl;
	
	flush();
	
	message->send_body( this );
	
	return true;
			
	//write( &request->body()[ 0 ], request->body().size() );
	
//	*this << http::endl;
	
//	return flush();
			
	//*this << flush;
	/*
	msg = m_ostream.str();
			
	nklog( log::verbose, "%s", msg.c_str() );
			
	ret = ( int ) m_socket->send( ( buf_t ) msg.c_str(), msg.size() );
	
			
	if ( ret != msg.size() )
	{
		response::ptr response = new http::response( -1 );
		
		nklog( log::error, "send failed: %d(%d)", platform::error() );
		m_reply( response );
		m_reply = NULL;
	}
	*/
}


bool
connection::flush()
{
	std::string msg = m_ostream.str();
	ssize_t		num = 0;
		
	if ( msg.size() > 0 )
	{
		nklog( log::verbose, "sending msg: %s", msg.c_str() );
		num = send( reinterpret_cast< const uint8_t* >( msg.c_str() ), msg.size() );
		m_ostream.str( "" );
		m_ostream.clear();
	}
		
	return ( msg.size() == num ) ? true : false;
}


void
connection::close()
{
}


ssize_t
connection::process()
{
	uint8_t	buf[ 4192 ];
	ssize_t	num;

	while ( 1 )
	{
		memset( buf, 0, sizeof( buf ) );
		num = recv( ( std::uint8_t* ) buf, sizeof( buf ) );

		if ( num > 0 )
		{
			size_t processed;

			processed = http_parser_execute( m_parser, m_settings, ( const char* ) buf, num );

			if ( processed != num )
			{
				nklog( log::error, "http_parser_execute() failed: bytes read = %ld, processed = %ld",
									num, processed );
				close();
				break;
			}
			
			if ( m_parser->upgrade )
			{
			}
		}
		else if ( num < 0 )
		{
			if ( platform::error() != ( int ) socket::error::would_block )
			{
				close();
			}

			break;
		}
		else
		{
			close();
			http_parser_execute( m_parser, m_settings, NULL, 0 );
			break;
		}
	}
	
	return num;
}





/*
void
connection::shutdown()
{
	runloop::instance()->unregisterSocket( this );

#undef close
	close();
	
	if ( m_closure )
	{
		socket::ptr temp( this );

		m_closure( temp );
	}
}
*/


int
connection::message_will_begin( http_parser *parser )
{
	connection *self = reinterpret_cast< connection* >( parser->data );
	
	nklog( log::verbose, "reading new message" );

	self->m_parser->upgrade = 0;
	self->m_parse_state	= NONE;
	
	self->m_uri_value.clear();
	self->m_header_field.clear();
	self->m_header_value.clear();
	
	self->m_handler	= NULL;
	self->m_request = NULL;
	
	return 0;
}


int
connection::uri_was_received( http_parser *parser, const char *buf, size_t len )
{
	connection	*self = reinterpret_cast< connection* >( parser->data );
	std::string	str( buf, len );
	
	nklog( log::verbose, "uri was received: %s", str.c_str() );
	
	self->m_uri_value.append( str );
	
	return 0;
}


bool
connection::resolve( http_parser *parser )
{
	bool resolved = false;
	
	m_method = parser->method;
	
	auto it = m_handlers.find( parser->method );
	
	if ( it != m_handlers.end() )
	{
		for ( handler::list::iterator it2 = it->second.begin(); it2 != it->second.end(); it2++ )
		{
			std::regex regex( ( *it2 )->m_path );
			
			if ( std::regex_search( m_uri_value, regex ) )
			{
				m_handler = *it2;
				break;
			}
		}
	}
	
	if ( !m_handler )
	{
		nklog( log::error, "unable to find handler for method %d -> %s", m_method, m_uri_value.c_str() );
		goto exit;
	}
	
	if ( m_handler->m_rwb )
	{
		m_request = m_handler->m_rwb();
	}
	else
	{
		m_request = new http::request( parser->method, new uri( m_uri_value ) );
	}

	if ( !m_request )
	{
		nklog( log::error, "unable to create request for method %d -> %s", m_method, m_uri_value.c_str() );
		goto exit;
	}
	
	resolved = true;

exit:

	return resolved;
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
			if ( !self->m_handler )
			{
				if ( !self->resolve( parser ) )
				{
					response::ptr response = new http::response( 404 );
					response->add_to_header( "Connection", "Close" );
					response->add_to_header( "Content-Type", "text/html" );
					*response << "<html>Error 404: Content Not Found</html>";
					self->put( response );
					ret = -1;
					goto exit;
				}
			}
			
			self->m_request->add_to_header( self->m_header_field, self->m_header_value );
		}
		
		self->m_header_field.assign( str );
		self->m_parse_state = FIELD;
	}
	else
	{
		self->m_header_field.append( str );
	}

exit:

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
	
	return 0;
}

	
int
connection::headers_were_received( http_parser *parser )
{
	connection			*self	= reinterpret_cast< connection* >( parser->data );
	handlers::iterator	it;
	int					ret		= 0;
	
	fprintf( stderr, "headers were received\n" );
	
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

	return ret;
}

	
int
connection::body_was_received( http_parser *parser, const char *buf, size_t len )
{
	connection *self = reinterpret_cast< connection* >( parser->data );
	
	fprintf( stderr, "body_was_received\n" );
	
	self->m_handler->m_rbwr( self->m_request, ( const uint8_t* ) buf, len );
	
	return 0;
}

	
int
connection::message_was_received( http_parser *parser )
{
	connection *self = reinterpret_cast< connection* >( parser->data );
	
	fprintf( stderr, "message was received\n" );
	
	self->m_handler->m_r( self->m_request, [=]( response::ptr response )
	{
		self->put( response );
	} );
	
	return 0;
}


#if defined( __APPLE__ )
#	pragma mark client implementation
#endif

client::client( const request::ptr &request, auth_f auth_func, response_f response_func )
:
	m_request( request ),
	m_auth_func( auth_func ),
	m_response_func( response_func )
{
}


client::~client()
{
}


const std::string&
string( status code )
{
	switch ( code )
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
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::ok:
		{
			static std::string s( "" );
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
			static std::string s( "Error" );
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
			static std::string s( "Error" );
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
