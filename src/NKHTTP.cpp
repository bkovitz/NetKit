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

using namespace netkit::http;


#if defined(WIN32)

#	include <winspool.h>
#	include <limits.h>
#	include <sys/types.h>
#	include <string.h>
#	define strcasecmp _stricmp

/*
 * Appends src to string dst of size siz (unlike strncat, siz is the
 * full size of dst, not space left).  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz <= strlen(dst)).
 * Returns strlen(initial dst) + strlen(src); if retval >= siz,
 * truncation occurred.
 */
static size_t
strlcat( char *dst, const char *src, size_t siz)
{
	register char *d = dst;
	register const char *s = src;
	register size_t n = siz;
	size_t dlen;

	/* Find the end of dst and adjust bytes left but don't go past end */
	while (n-- != 0 && *d != '\0')
		d++;
	dlen = d - dst;
	n = siz - dlen;

	if (n == 0)
		return(dlen + strlen(s));
	while (*s != '\0') {
		if (n != 1) {
			*d++ = *s;
			n--;
		}
		s++;
	}
	*d = '\0';

	return(dlen + (s - src));	/* count does not include NUL */
}
#endif


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
connection::put( const message::ptr &message )
{
	//message->send_prologue( this );

//	*this << request->method() << " " << request->uri()->path() << " HTTP/1.1" << http::endl;
	// *this << "Host: " << request->uri()->host() << endl;

	for ( message::header::const_iterator it = message->heade().begin(); it != message->heade().end(); it++ )
	{
		*this << it->first << ": " << it->second << endl;
	}
			
	*this << http::endl;
	
//	message->send_body( this );
			
	//write( &request->body()[ 0 ], request->body().size() );
	
	*this << http::endl;
	
	return flush();
			
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
		//num = m_socket->send( reinterpret_cast< const uint8_t* >( msg.c_str() ), msg.size() );
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
		num = read( ( buf_t ) buf, sizeof( buf ) );

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


int
connection::header_field_was_received( http_parser *parser, const char *buf, size_t len )
{
	connection	*self = reinterpret_cast< connection* >( parser->data );
	int			ret = 0;
	std::string	str( buf, len );
	
	
	//if ( !self->m_request && !self->build_message() )
	if ( !self->m_request )
	{
		ret = -1;
		goto exit;
	}
	
	if ( self->m_parse_state != FIELD )
	{
		if ( ( self->m_header_field.size() > 0 ) && ( self->m_header_value.size() > 0 ) )
		{
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
	
	self->m_method = parser->method;
	
	it = m_handlers.find( parser->method );
	
	if ( it != m_handlers.end() )
	{
		for ( handler::list::iterator it2 = it->second.begin(); it2 != it->second.begin(); it2++ )
		{
			std::regex regex( ( *it2 )->m_path );
			
			if ( std::regex_search( self->m_uri_value, regex ) )
			{
				self->m_handler = *it2;
				break;
			}
		}
	}
	
	if ( !self->m_handler )
	{
		nklog( log::error, "unable to find handler for method %d -> %s", self->m_method, self->m_uri_value.c_str() );
		ret = -1;
		goto exit;
	}
	
	self->m_request = self->m_handler->m_rwb();

	if ( !self->m_request )
	{
		nklog( log::error, "unable to create request for method %d -> %s", self->m_method, self->m_uri_value.c_str() );
		ret = -1;
		goto exit;
	}
	
	if ( ( self->m_header_field.size() > 0 ) && ( self->m_header_value.size() > 0 ) )
	{
		self->m_request->add_to_header( self->m_header_field, self->m_header_value );
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
	} );
	
	return 0;
}


#if defined( __APPLE__ )
#	pragma mark client implementation
#endif
#if 0
client::client()
:
	m_reply( NULL )
{
	set_response_will_begin_handler( []( int code )
	{
		return response::ptr( new http::response( code ) );
	} );
	
	set_message_was_received_handler( [this]( message::ptr message ) -> int
	{
		http::response::ptr response = dynamic_pointer_cast< http::response >( message );
		
		if ( m_reply )
		{
			m_reply( response );
		}
		
		return 0;
	} );
}


client::~client()
{
}


void
client::send( const request::ptr &request, reply reply )
{
	if ( m_reply )
	{
		response::ptr response = new http::response( -1 );
		reply( response );
		goto exit;
	}

	m_reply = reply;

	if ( !m_socket->is_open() || !m_uri || ( request->uri()->host() != m_uri->host() ) || ( request->uri()->port() != m_uri->port() ) )
	{
		m_socket->close();
		
		if ( m_socket->open() != 0 )
		{
		}
		
		m_uri = request->uri();

		ip::address::resolve( request->uri()->host(), request->uri()->port(), [&]( int status, ip::address::list addresses )
		{
			if ( status == 0 )
			{
				m_socket->connect( addresses[ 0 ], [&]( int status )
				{
					if ( status == 0 )
					{
						if ( ( request->uri()->scheme() == TEXT( "http" ) ) || ( m_socket->tls_connect() == 0 ) )
						{
							connection::send( request );
						}
						else
						{
							response::ptr response = new http::response( -1 );
							nklog( log::error, "unable to setup tls" );
							m_reply( response );
						}
					}
				} );
				
			}
		} );
	}
	else
	{
		connection::send( request );
	}
	
exit:

	return;
}

#if defined( __APPLE__ )
#	pragma mark server implementation
#endif

service::service()
{
}


service::~service()
{
}


#endif


#if defined( __APPLE__ )
#	pragma mark message implementation
#endif

message::message()
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
	*conn << m_method << " " << m_uri->path() << " HTTP/1.1\r\n";
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

		case status::switchingProtocols:
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

		case status::notAuthoritative:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::noContent:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::resetContent:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::partialContent:
		{
			static std::string s( "Error" );
			return s;
		}
		break;


		case status::multipleChoices:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::movedPermanently:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::movedTemporarily:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::seeOther:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::notModified:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::useProxy:
		{
			static std::string s( "Error" );
			return s;
		}
		break;


		case status::badRequest:
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

		case status::paymentRequired:
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

		case status::notFound:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::methodNotAllowed:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::notAcceptable:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::proxyAuthentication:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::requestTimeout:
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

		case status::lengthRequired:
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

		case status::requestTooLarge:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::uriTooLong:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::unsupportedMediaType:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::requestedRange:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::expectationFailed:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::upgradeRequired:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::serverError:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::notImplemented:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::badGateway:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::serviceUnavailable:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::gatewayTimeout:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::notSupported:
		{
			static std::string s( "Error" );
			return s;
		}
		break;


		case status::authorizedCancelled:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::pkiError:
		{
			static std::string s( "Error" );
			return s;
		}
		break;

		case status::webifDisabled:
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
