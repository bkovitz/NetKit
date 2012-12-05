#ifndef _CoreApp_http_h
#define _CoreApp_http_h

#include <CoreApp/connection.h>
#include <CoreApp/tcp_socket.h>
#include <CoreApp/server.h>
#include <CoreApp/tstring.h>
#include <CoreApp/uri.h>
#include <CoreApp/cstring.h>
#include <vector>
#include <list>

struct http_parser_settings;
struct http_parser;

namespace CoreApp {
namespace http {

class message;
typedef smart_ptr< message > message_ptr;

class request;
typedef smart_ptr< request > request_ptr;

class response;
typedef smart_ptr< response > response_ptr;

class connection : public CoreApp::connection< tcp::client >
{
public:

	typedef std::function< request_ptr ( const CoreApp::uri::ptr& ) >	request_will_begin_handler;
	typedef std::function< response_ptr ( int ) >						response_will_begin_handler;
	typedef std::function< int ( message_ptr& )	>						headers_were_received_handler;
	typedef std::function< int ( message_ptr&, const blob& ) >			body_was_received_handler;
	typedef std::function< int ( message_ptr& )	>						message_was_received_handler;

	typedef smart_ptr< connection > ptr;
	
public:

	connection( int type );

	connection( const tcp::client::ptr &sock, int type );
	
	virtual ~connection();

	void
	set_request_will_begin_handler( request_will_begin_handler handler );
	
	void
	set_response_will_begin_handler( response_will_begin_handler handler );
	
	void
	set_headers_were_received_handler( headers_were_received_handler handler );
	
	void
	set_body_was_received_handler( body_was_received_handler handler );
	
	void
	set_message_was_received_handler( message_was_received_handler handler );
	
	int
	http_major() const;
	
	int
	http_minor() const;
	
	bool
	send( const message_ptr &message );
	
protected:

	typedef CoreApp::connection< tcp::client >	super;
	
	typedef std::vector< std::string >			strings;
	
	virtual void
	can_send_data();

	virtual void
	can_recv_data();

	request_will_begin_handler			m_request_will_begin_handler;
	response_will_begin_handler			m_response_will_begin_handler;
	headers_were_received_handler		m_headers_were_received_handler;
	body_was_received_handler			m_body_was_received_handler;
	message_was_received_handler		m_message_was_received_handler;
	
	std::tstring						m_uri_value;
	std::tstring						m_header_field;
	std::tstring						m_header_value;
	int									m_operation;
	time_t								m_start;
	bool								m_okay;
	blob								m_body;
	
	enum
	{
		NONE = 0,
		FIELD,
		VALUE
	};
	
	http_parser_settings		*m_settings;
	http_parser					*m_parser;
	int							m_parse_state;

	message_ptr					m_message;

	std::string					m_expect;
	std::string					m_host;
	std::string					m_authorization;
	std::string					m_username;
	std::string					m_password;
	
private:

	static int
	message_will_begin( http_parser *parser );

	static int
	uri_was_received( http_parser *parser, const char *buf, size_t len );

	static int
	header_field_was_received( http_parser *parser, const char *buf, size_t len );

	static int
	header_value_was_received( http_parser *parser, const char *buf, size_t len );
	
	static int
	headers_were_received( http_parser *parser );
	
	static int
	body_was_received( http_parser *parser, const char *buf, size_t len );
	
	static int
	message_was_received( http_parser *parser );
	
	bool
	build_message();
	
	void
	init( int type );
};


class client : public connection
{
public:

	typedef std::function< void ( const response_ptr &response ) > reply;

	typedef smart_ptr< client > ptr;
	
	client();
	
	virtual ~client();
	
	void
	send( const request_ptr &request, reply reply );
	
protected:

	uri::ptr	m_uri;
	reply		m_reply;
};


class server : public CoreApp::server
{
public:

	class handler
	{
	public:
	
		virtual request_ptr
		message_will_begin( const uri::ptr &uri ) = 0;
	
		virtual int
		headers_were_received( connection::ptr conn, request_ptr &request ) = 0;
		
		virtual int
		message_was_received( connection::ptr conn, request_ptr &request ) = 0;
	};
	
public:

	typedef smart_ptr< server > ptr;
	
	server();
	
	virtual ~server();
	
	void
	set_handler( const std::tstring &path, handler *handler );

protected:

	typedef std::deque< std::pair< std::tstring, handler* > > handlers;

	typedef std::list< connection::ptr > connections;
	
	virtual bool
	adopt( const socket::ptr &sock, uint8_t *peek, size_t len );
	
	connections m_connections;
	handlers	m_handlers;
};


class message : public object
{
public:

	typedef std::list< std::pair< std::tstring, std::tstring > > header;
	
public:

	typedef smart_ptr< message > ptr;
	
public:

	message();
	
	virtual ~message();
	
	inline const header&
	heder() const
	{
		return m_header;
	}
	
	virtual void
	add_to_header( const header& header );

	virtual void
	add_to_header( const std::tstring &key, int val );

	virtual void
	add_to_header( const std::tstring &key, const std::tstring &val );
	
	inline const std::tstring&
	content_type() const
	{
		return m_content_type;
	}
	
	inline size_t
	content_length() const
	{
		return m_content_length;
	}
	
	virtual void
	write( const uint8_t *buf, size_t len );
	
	inline std::string
	body() const
	{
		return m_ostream.str();
	}
	
	template < class U >
	inline message&
	operator<<( U t )
	{
		m_ostream << t;
		return *this;
	}
	
	inline message&
	operator<<( message& ( *func )( message& ) )
	{
		return func( *this );
	}
	
	virtual void
	send_prologue( connection::ptr conn ) const = 0;
	
	virtual bool
	send_body( connection::ptr conn ) const;
	
protected:

	header				m_header;
	std::tstring		m_content_type;
	size_t				m_content_length;
	std::ostringstream	m_ostream;
};


class request : public message
{
public:

	typedef smart_ptr< request > ptr;

	inline request( const std::string &uri )
	:
		m_uri( new CoreApp::uri( uri ) ),
		m_context( NULL )
	{
		init();
	}

	inline request( const uri::ptr &uri )
	:
		m_uri( uri ),
		m_context( NULL )
	{
		init();
	}
	
	inline request( const request &r )
	:
		m_uri( r.m_uri ),
		m_method( r.m_method ),
		m_context( r.m_context )
	{
		init();
	}

	virtual ~request();
	
	inline const uri::ptr&
	uri() const
	{
		return m_uri;
	}
	
	inline std::string
	method() const
	{
		return m_method;
	}
	
	inline void
	set_method( const std::string &method )
	{
		m_method = method;
	}
	
	inline void*
	context() const
	{
		return m_context;
	}
	
	inline void
	set_context( void *context )
	{
		m_context = context;
	}
	
	virtual void
	send_prologue( connection::ptr conn ) const;
	
private:

	void
	init();

	std::string			m_method;
	CoreApp::uri::ptr	m_uri;
	void				*m_context;
};


class response : public message
{
public:

	typedef smart_ptr< response > ptr;

	response();
	
	response( int status );

	virtual ~response();

	inline int
	status() const
	{
		return m_status;
	}

	inline void
	set_status( int status )
	{
		m_status = status;
	}
	
	virtual void
	send_prologue( connection::ptr conn ) const;

private:

	void
	init();

	int	m_status;
};

namespace status {

const std::tstring&
string( int code );

enum
{
	error				= -1,		/* An error response from httpXxxx() */
	cont				= 100,		/* Everything OK, keep going... */
	switchingProtocols,				/* HTTP upgrade to TLS/SSL */

	ok					= 200,		/* OPTIONS/GET/HEAD/POST/TRACE command was successful */
	created,						/* PUT command was successful */
	accepted,						/* DELETE command was successful */
	notAuthoritative,				/* Information isn't authoritative */
	noContent,						/* Successful command, no new data */
	resetContent,					/* Content was reset/recreated */
	partialContent,					/* Only a partial file was recieved/sent */

	multipleChoices		= 300,		/* Multiple files match request */
	movedPermanently,				/* Document has moved permanently */
	movedTemporarily,				/* Document has moved temporarily */
	seeOther,						/* See this other link... */
	notModified,					/* File not modified */
	useProxy,						/* Must use a proxy to access this URI */

	badRequest			= 400,		/* Bad request */
	unauthorized,					/* Unauthorized to access host */
	paymentRequired,				/* Payment required */
	forbidden,						/* Forbidden to access this URI */
	notFound,						/* URI was not found */
	methodNotAllowed,				/* Method is not allowed */
	notAcceptable,					/* Not Acceptable */
	proxyAuthentication,			/* Proxy Authentication is Required */
	requestTimeout,					/* Request timed out */
	conflict,						/* Request is self-conflicting */
	gone,							/* Server has gone away */
	lengthRequired,					/* A content length or encoding is required */
	precondition,					/* Precondition failed */
	requestTooLarge,				/* Request entity too large */
	uriTooLong,						/* URI too long */
	unsupportedMediaType,			/* The requested media type is unsupported */
	requestedRange,					/* The requested range is not satisfiable */
	expectationFailed,				/* The expectation given in an Expect header field was not met */
	upgradeRequired		= 426,		/* Upgrade to SSL/TLS required */

	serverError			= 500,		/* Internal server error */
	notImplemented,					/* Feature not implemented */
	badGateway,						/* Bad gateway */
	serviceUnavailable,				/* Service is unavailable */
	gatewayTimeout,					/* Gateway connection timed out */
	notSupported,					/* HTTP version not supported */

	authorizedCancelled	= 1000,		/* User canceled authorization @since CUPS 1.4@ */
	pkiError,						/* Error negotiating a secure connection @since CUPS 1.5/Mac OS X 10.7@ */
	webifDisabled					/* Web interface is disabled @private@ */
};

}

inline CoreApp::connection< tcp::client >&
endl( CoreApp::connection< tcp::client > &conn )
{
	conn << "\r\n";
	return conn;
}

}
}

#endif
