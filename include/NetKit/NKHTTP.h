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
 
#ifndef _netkit_http_h
#define _netkit_http_h

#include <NetKit/NKTCPSocket.h>
#include <NetKit/NKProxy.h>
#include <NetKit/NKURI.h>
#include <NetKit/cstring.h>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <map>

struct http_parser_settings;
struct http_parser;

namespace netkit {

namespace http {

class connection;
typedef smart_ptr< connection > connection_ptr;

struct method
{
	static std::string
	as_string( std::uint8_t val );
	
	static const std::uint8_t delet;
	static const std::uint8_t get;
	static const std::uint8_t head;
	static const std::uint8_t post;
	static const std::uint8_t put;
	static const std::uint8_t connect;
	static const std::uint8_t options;
	static const std::uint8_t trace;
	static const std::uint8_t copy;
	static const std::uint8_t lock;
	static const std::uint8_t mkcol;
	static const std::uint8_t move;
	static const std::uint8_t propfind;
	static const std::uint8_t proppatch;
	static const std::uint8_t search;
	static const std::uint8_t unlock;
	static const std::uint8_t report;
	static const std::uint8_t mkactivity;
	static const std::uint8_t checkout;
	static const std::uint8_t merge;
	static const std::uint8_t msearch;
	static const std::uint8_t notify;
	static const std::uint8_t subscribe;
	static const std::uint8_t unsubscribe;
	static const std::uint8_t patch;
	static const std::uint8_t purge;
};

enum class status
{
	error					= -1,
	cont					= 100,
	switching_protocols,
	ok						= 200,
	created,
	accepted,
	not_authoritative,
	no_content,
	reset_content,
	partial_content,

	multiple_choices		= 300,
	moved_permanently,
	moved_temporarily,
	see_other,
	not_modified,
	use_proxy,

	bad_request				= 400,
	unauthorized,
	payment_required,
	forbidden,
	not_found,
	method_not_allowed,
	not_acceptable,
	proxy_authentication,
	request_timeout,
	conflict,
	gone,
	length_required,
	precondition,
	request_too_large,
	uri_too_long,
	unsupported_media_type,
	requested_range,
	expectation_failed,
	upgrade_required		= 426,

	server_error			= 500,
	not_implemented,
	bad_gateway,
	service_unavailable,
	gateway_timeout,
	not_supported,

	authorized_cancelled	= 1000,
	pki_error,
	webif_disabled
};

class message : public object
{
public:

	typedef std::list< std::pair< std::string, std::string > > header;
	
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
	add_to_header( const std::string &key, int val );

	virtual void
	add_to_header( const std::string &key, const std::string &val );
	
	virtual void
	remove_from_header( const std::string &key );
	
	inline const std::string&
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
	send_prologue( connection_ptr conn ) const = 0;
	
	virtual bool
	send_body( connection_ptr conn ) const;
	
protected:

	header				m_header;
	std::string			m_content_type;
	size_t				m_content_length;
	std::ostringstream	m_ostream;
};


class request : public message
{
public:

	typedef smart_ptr< request > ptr;

	request( int method, const uri::ptr &uri );
	
	request( const request &r );

	virtual ~request();
	
	inline int
	method() const
	{
		return m_method;
	}
	
	inline const uri::ptr&
	uri() const
	{
		return m_uri;
	}
	
	inline const proxy::ptr&
	proxy() const
	{
		return m_proxy;
	}
	
	inline void
	set_proxy( const proxy::ptr &proxy )
	{
		m_proxy = proxy;
	}
		
	virtual void
	send_prologue( connection_ptr conn ) const;
	
	inline int32_t
	tries() const
	{
		return m_tries;
	}
	
	inline void
	new_try()
	{
		++m_tries;
	}
	
private:

	void
	init();

	int				m_method;
	uri::ptr		m_uri;
	proxy::ptr		m_proxy;
	std::int32_t	m_tries;
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
	send_prologue( connection_ptr conn ) const;

private:

	void
	init();

	int	m_status;
};


typedef std::function< void ( http::response::ptr response ) >										response_f;
typedef std::function< http::request::ptr (void) >													request_will_begin_f;
typedef std::function< void ( http::request::ptr request, const std::uint8_t *buf, size_t len ) >	request_body_was_received_f;
typedef std::function< void ( http::request::ptr request, response_f func ) >						request_f;

	
class connection : public sink
{
public:

	typedef smart_ptr< connection > ptr;
	
public:

	static sink::ptr
	adopt( source::ptr source, const std::uint8_t *buf, size_t len );
	
	static void
	bind( std::uint8_t method, const std::string &path, const std::string &type, request_f r );
	
	static void
	bind( std::uint8_t method, const std::string &path, const std::string &type, request_body_was_received_f rbwr, request_f r );
	
	static void
	bind( std::uint8_t method, const std::string &path, const std::string &type, request_will_begin_f rwb, request_body_was_received_f rbwr, request_f r );
	
	virtual ssize_t
	process();
	
	bool
	put( message::ptr message );
	
	int
	http_major() const;
	
	int
	http_minor() const;
	
	template < class U >
	inline connection&
	operator<<( U t )
	{
		m_ostream << t;
		return *this;
	}
	
	inline connection&
	operator<<( connection& ( *func )( connection& ) )
	{
		return func( *this );
	}
	
	bool
	flush();
	
	void
	close();
	
protected:

	class handler : public object
	{
	public:
	
		typedef smart_ptr< handler > ptr;
		typedef std::list< ptr > list;
		
		handler( const std::string &path, const std::string &type, request_f r )
		:
			m_path( path ),
			m_type( type ),
			m_r( r )
		{
		}
	
		handler( const std::string &path, const std::string &type, request_body_was_received_f rbwr, request_f r )
		:
			m_path( path ),
			m_type( type ),
			m_rbwr( rbwr ),
			m_r( r )
		{
		}
	
		handler( const std::string &path, const std::string &type, request_will_begin_f rwb, request_body_was_received_f rbwr, request_f r )
		:
			m_path( path ),
			m_type( type ),
			m_rwb( rwb ),
			m_rbwr( rbwr ),
			m_r( r )
		{
		}
	
		std::string					m_path;
		std::string					m_type;
		request_will_begin_f		m_rwb;
		request_body_was_received_f	m_rbwr;
		request_f					m_r;
	};
	
	typedef std::map< std::uint8_t, handler::list > handlers;
	
	enum
	{
		NONE = 0,
		FIELD,
		VALUE
	};
	
protected:

	connection( const source::ptr &source );
	
	virtual ~connection();
	
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
	
	static void
	bind( std::uint8_t method, handler::ptr handler );
	
	bool
	resolve( http_parser *parser );
	
	static handlers				m_handlers;
	
	std::uint8_t				m_method;
	std::string					m_uri_value;
	std::string					m_header_field;
	std::string					m_header_value;
	handler::ptr				m_handler;
	time_t						m_start;
	bool						m_okay;
	std::vector< std::uint8_t >	m_body;
	
	http_parser_settings		*m_settings;
	http_parser					*m_parser;
	int							m_parse_state;
	
	request::ptr				m_request;

	std::string					m_expect;
	std::string					m_host;
	std::string					m_authorization;
	std::string					m_username;
	std::string					m_password;
	
	std::ostringstream			m_ostream;
};


class client : public object
{
public:

	typedef smart_ptr< client >													ptr;
	typedef std::function< bool ( request::ptr &request, uint32_t status ) >	auth_f;
	typedef std::function< void ( uint32_t error, response::ptr response ) >	response_f;
	
	static request::ptr
	request( int method, const uri::ptr &uri );
	
	static void
	send( const request::ptr &request, response_f response_func );
	
	static void
	send( const request::ptr &request, auth_f auth_func, response_f response_func );
	
protected:

	client( const request::ptr &request, auth_f auth_func, response_f response_func );
	
	virtual ~client();
	
	request::ptr	m_request;
	response::ptr	m_response;
	auth_f			m_auth_func;
	response_f		m_response_func;
};




inline connection&
endl( connection &conn )
{
	conn << "\r\n";
	return conn;
}

}

}

#endif
