#include "http.h"
#include "socket.h"
#include "base64.h"
#include "cstring.h"
#include "os.h"
#include "log.h"
#include <http_parser.h>
#include <fstream>
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

using namespace CoreApp::http;


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

connection::connection( int type )
:
	m_okay( true )
{
	init( type );
}


connection::connection( const tcp::client::ptr &sock, int type )
:
	super( sock ),
	m_okay( true )
{
	init( type );
}


void
connection::init( int type )
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
	
	http_parser_init( m_parser, ( http_parser_type ) type );
	m_parser->data = this;
	
	m_request_will_begin_handler	= NULL;
	m_response_will_begin_handler	= NULL;
	m_headers_were_received_handler	= NULL;
	m_body_was_received_handler		= NULL;
	m_message_was_received_handler	= NULL;
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
connection::set_request_will_begin_handler( request_will_begin_handler handler )
{
	m_request_will_begin_handler = handler;
}


void
connection::set_response_will_begin_handler( response_will_begin_handler handler )
{
	m_response_will_begin_handler = handler;
}


void
connection::set_headers_were_received_handler( headers_were_received_handler handler )
{
	m_headers_were_received_handler = handler;
}
	
	
void
connection::set_body_was_received_handler( body_was_received_handler handler )
{
	m_body_was_received_handler = handler;
}

	
void
connection::set_message_was_received_handler( message_was_received_handler handler )
{
	m_message_was_received_handler = handler;
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
connection::send( const message::ptr &message )
{
	message->send_prologue( this );

//	*this << request->method() << " " << request->uri()->path() << " HTTP/1.1" << http::endl;
	// *this << "Host: " << request->uri()->host() << endl;

	for ( message::header::const_iterator it = message->heder().begin(); it != message->heder().end(); it++ )
	{
		*this << it->first << ": " << it->second << endl;
	}
			
	*this << http::endl;
	
	message->send_body( this );
			
	//write( &request->body()[ 0 ], request->body().size() );
	
	*this << http::endl;
	
	return flush();
			
	//*this << flush;
	/*
	msg = m_ostream.str();
			
	netlog( log::verbose, "%s", msg.c_str() );
			
	ret = ( int ) m_socket->send( ( buf_t ) msg.c_str(), msg.size() );
	
			
	if ( ret != msg.size() )
	{
		response::ptr response = new http::response( -1 );
		
		netlog( log::error, "send failed: %d(%d)", os::error() );
		m_reply( response );
		m_reply = NULL;
	}
	*/
}


void
connection::can_send_data()
{
}


void
connection::can_recv_data()
{
	uint8_t	buf[ 4192 ];
	ssize_t	num;

	while ( 1 )
	{
		memset( buf, 0, sizeof( buf ) );
		num = recv( ( buf_t ) buf, sizeof( buf ) );

		if ( num > 0 )
		{
			size_t processed;

			processed = http_parser_execute( m_parser, m_settings, ( const char* ) buf, num );

			if ( processed != num )
			{
				netlog( log::error, "http_parser_execute() failed: bytes read = %ld, processed = %ld",
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
			if ( os::error() != socket::error::wouldblock )
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
	
	netlog( log::verbose, "reading new message" );

	self->m_parser->upgrade = 0;
	self->m_parse_state	= NONE;
	
	self->m_uri_value.clear();
	self->m_header_field.clear();
	self->m_header_value.clear();
	
	self->m_message = NULL;
	
	return 0;
}


int
connection::uri_was_received( http_parser *parser, const char *buf, size_t len )
{
	connection		*self = reinterpret_cast< connection* >( parser->data );
	std::tstring	str( buf, len );
	
	netlog( log::verbose, "uri was received: %s", str.utf8().c_str() );
	
	self->m_uri_value.append( str );
	
	return 0;
}


int
connection::header_field_was_received( http_parser *parser, const char *buf, size_t len )
{
	connection		*self = reinterpret_cast< connection* >( parser->data );
	int				ret = 0;
	std::tstring	str( buf, len );
	
	if ( !self->m_message && !self->build_message() )
	{
		ret = -1;
		goto exit;
	}
	
	if ( self->m_parse_state != FIELD )
	{
		if ( ( self->m_header_field.size() > 0 ) && ( self->m_header_value.size() > 0 ) )
		{
			self->m_message->add_to_header( self->m_header_field, self->m_header_value );
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
	connection		*self = reinterpret_cast< connection* >( parser->data );
	std::tstring	str( buf, len );
	
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
	connection	*self	= reinterpret_cast< connection* >( parser->data );
	int			ret		= 0;
	
	fprintf( stderr, "headers were received\n" );
	
	if ( !self->m_message && !self->build_message() )
	{
		ret = -1;
		goto exit;
	}
	
	if ( ( self->m_header_field.size() > 0 ) && ( self->m_header_value.size() > 0 ) )
	{
		self->m_message->add_to_header( self->m_header_field, self->m_header_value );
	}
	
	if ( self->m_headers_were_received_handler )
	{
		self->m_headers_were_received_handler( self->m_message );
	}
		
		/*
	for ( header::const_iterator it = self->m_message->header().begin(); it != self->m_message->header().end(); it++ )
	{
		netlog( log::verbose, "%s %s\n", it->first.c_str(), it->second.c_str() );
	
		if ( it->first == TEXT( "Content-Length" ) )
		{
			m_message->set_content_length( atoi( it->second.c_str() ) );
		}
		else if ( it->first == TEXT( "Content-Type" ) )
		{
			m_message->set_content_type( it->secoond );
		}
		else if ( it->first == "Host" )
		{
			m_message->set_host( it->second );
		}
		else if ( it->first == "Expect" )
		{
			m_message->set_expect( it->second );
		}
		else if ( *it1 == "Authorization" )
		{
			std::tstring base64_encoded;
			std::tstring decoded;
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
	self->m_message->write( reinterpret_cast< const uint8_t* >( buf ), len );
	
	return 0;
}

	
int
connection::message_was_received( http_parser *parser )
{
	connection	*self = reinterpret_cast< connection* >( parser->data );
	int			ret = 0;
	
	fprintf( stderr, "message was received\n" );
	if ( self->m_message_was_received_handler )
	{
		ret = self->m_message_was_received_handler( self->m_message );
	}
	
	return ret;
}


bool
connection::build_message()
{
	assert( !m_message );
	
	switch ( m_parser->type )
	{
		case HTTP_REQUEST:
		{
			if ( m_request_will_begin_handler )
			{
				m_message = m_request_will_begin_handler( new uri( m_uri_value ) );
			}
		}
		break;
			
		case HTTP_RESPONSE:
		{
			if ( m_response_will_begin_handler )
			{
				m_message = m_response_will_begin_handler( m_parser->status_code );
			}
		}
		break;
	}
	
	return ( m_message ) ? true : false;
}

#if defined( __APPLE__ )
#	pragma mark client implementation
#endif

client::client()
:
	connection( HTTP_RESPONSE ),
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
							netlog( log::error, "unable to setup tls" );
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

server::server()
{
}


server::~server()
{
}


bool
server::adopt( const CoreApp::socket::ptr &sock, uint8_t *buf, size_t len )
{
	tcp::client::ptr		tcp_sock = dynamic_pointer_cast< tcp::client >( sock );
	http::connection::ptr	conn;
	
	if ( tcp_sock )
	{
		if ( ( len >= 3 ) &&
			 ( ( std::strncasecmp( ( const char* ) buf, "get", 3 ) == 0 ) ||
		       ( std::strncasecmp( ( const char* ) buf, "pos", 3 ) == 0 ) ||
		       ( std::strncasecmp( ( const char* ) buf, "opt", 3 ) == 0 ) ||
		       ( std::strncasecmp( ( const char* ) buf, "hea", 3 ) == 0 ) ) )
		{
			conn = new connection( tcp_sock, HTTP_REQUEST );
	
			if ( conn )
			{
				conn->set_request_will_begin_handler( [ this, conn ]( const CoreApp::uri::ptr &uri ) -> request::ptr
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


void
server::set_handler( const std::tstring &path, handler *handler )
{
	m_handlers.push_back( std::make_pair( path, handler ) );
}

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
message::add_to_header( const std::tstring &key, int val )
{
	char buf[ 1024 ];
		
	std::sprintf_s( buf, sizeof( buf ), sizeof( buf ), "%d", val );
	m_header.push_back( make_pair( key, buf ) );
}


void
message::add_to_header( const std::tstring &key, const std::tstring &val )
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
message::send_body( connection::ptr conn ) const
{
	std::string body = m_ostream.str();
	
	conn->write( reinterpret_cast< const uint8_t* >( body.c_str() ), body.size() );
	
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
request::send_prologue( connection::ptr conn ) const
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


const std::tstring&
status::string( int code )
{
	switch ( code )
	{
		case http::status::error:
		{
			static std::tstring s( "Error" );
			return s;
		}
		break;

		case http::status::cont:
		{
			static std::tstring s( "Error" );
			return s;
		}
		break;

		case http::status::switchingProtocols:
		{
			static std::tstring s( "Error" );
			return s;
		}
		break;

		case http::status::ok:
		{
			static std::tstring s( "" );
			return s;
		}
		break;

		case http::status::created:
		{
			static std::tstring s( "Error" );
			return s;
		}
		break;

		case http::status::accepted:
		{
			static std::tstring s( "Error" );
			return s;
		}
		break;

		case http::status::notAuthoritative:
		{
			static std::tstring s( "Error" );
			return s;
		}
		break;

		case http::status::noContent:
		{
			static std::tstring s( "Error" );
			return s;
		}
		break;

		case http::status::resetContent:
		{
			static std::tstring s( "Error" );
			return s;
		}
		break;

		case http::status::partialContent:
		{
			static std::tstring s( "Error" );
			return s;
		}
		break;


		case http::status::multipleChoices:
		{
			static std::tstring s( "Error" );
			return s;
		}
		break;

		case http::status::movedPermanently:
		{
			static std::tstring s( "Error" );
			return s;
		}
		break;

		case http::status::movedTemporarily:
		{
			static std::tstring s( "Error" );
			return s;
		}
		break;

		case http::status::seeOther:
		{
			static std::tstring s( "Error" );
			return s;
		}
		break;

		case http::status::notModified:
		{
			static std::tstring s( "Error" );
			return s;
		}
		break;

		case http::status::useProxy:
		{
			static std::tstring s( "Error" );
			return s;
		}
		break;


		case http::status::badRequest:
		{
			static std::tstring s( "Error" );
			return s;
		}
		break;

		case http::status::unauthorized:
		{
			static std::tstring s( "Error" );
			return s;
		}
		break;

		case http::status::paymentRequired:
		{
			static std::tstring s( "Error" );
			return s;
		}
		break;

		case http::status::forbidden:
		{
			static std::tstring s( "Error" );
			return s;
		}
		break;

		case http::status::notFound:
		{
			static std::tstring s( "Error" );
			return s;
		}
		break;

		case http::status::methodNotAllowed:
		{
			static std::tstring s( "Error" );
			return s;
		}
		break;

		case http::status::notAcceptable:
		{
			static std::tstring s( "Error" );
			return s;
		}
		break;

		case http::status::proxyAuthentication:
		{
			static std::tstring s( "Error" );
			return s;
		}
		break;

		case http::status::requestTimeout:
		{
			static std::tstring s( "Error" );
			return s;
		}
		break;

		case http::status::conflict:
		{
			static std::tstring s( "Error" );
			return s;
		}
		break;

		case http::status::gone:
		{
			static std::tstring s( "Error" );
			return s;
		}
		break;

		case http::status::lengthRequired:
		{
			static std::tstring s( "Error" );
			return s;
		}
		break;

		case http::status::precondition:
		{
			static std::tstring s( "Error" );
			return s;
		}
		break;

		case http::status::requestTooLarge:
		{
			static std::tstring s( "Error" );
			return s;
		}
		break;

		case http::status::uriTooLong:
		{
			static std::tstring s( "Error" );
			return s;
		}
		break;

		case http::status::unsupportedMediaType:
		{
			static std::tstring s( "Error" );
			return s;
		}
		break;

		case http::status::requestedRange:
		{
			static std::tstring s( "Error" );
			return s;
		}
		break;

		case http::status::expectationFailed:
		{
			static std::tstring s( "Error" );
			return s;
		}
		break;

		case http::status::upgradeRequired:
		{
			static std::tstring s( "Error" );
			return s;
		}
		break;


		case http::status::serverError:
		{
			static std::tstring s( "Error" );
			return s;
		}
		break;

		case http::status::notImplemented:
		{
			static std::tstring s( "Error" );
			return s;
		}
		break;

		case http::status::badGateway:
		{
			static std::tstring s( "Error" );
			return s;
		}
		break;

		case http::status::serviceUnavailable:
		{
			static std::tstring s( "Error" );
			return s;
		}
		break;

		case http::status::gatewayTimeout:
		{
			static std::tstring s( "Error" );
			return s;
		}
		break;

		case http::status::notSupported:
		{
			static std::tstring s( "Error" );
			return s;
		}
		break;


		case http::status::authorizedCancelled:
		{
			static std::tstring s( "Error" );
			return s;
		}
		break;

		case http::status::pkiError:
		{
			static std::tstring s( "Error" );
			return s;
		}
		break;

		case http::status::webifDisabled:
		{
			static std::tstring s( "Error" );
			return s;
		}
		break;


		default:
		{
			static std::tstring s( "Error" );
			return s;
		}
		break;
	}
}
