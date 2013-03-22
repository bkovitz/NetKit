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
	to_string( std::uint8_t val );
	
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


struct status
{
	static std::string
	to_string( std::uint16_t val );
	
	static const std::uint16_t error;
	static const std::uint16_t cont;
	static const std::uint16_t switching_protocols;
	static const std::uint16_t ok;
	static const std::uint16_t created;
	static const std::uint16_t accepted;
	static const std::uint16_t not_authoritative;
	static const std::uint16_t no_content;
	static const std::uint16_t reset_content;
	static const std::uint16_t partial_content;
	static const std::uint16_t multiple_choices;
	static const std::uint16_t moved_permanently;
	static const std::uint16_t moved_temporarily;
	static const std::uint16_t see_other;
	static const std::uint16_t not_modified;
	static const std::uint16_t use_proxy;
	static const std::uint16_t bad_request;
	static const std::uint16_t unauthorized;
	static const std::uint16_t payment_required;
	static const std::uint16_t forbidden;
	static const std::uint16_t not_found;
	static const std::uint16_t method_not_allowed;
	static const std::uint16_t not_acceptable;
	static const std::uint16_t proxy_authentication;
	static const std::uint16_t request_timeout;
	static const std::uint16_t conflict;
	static const std::uint16_t gone;
	static const std::uint16_t length_required;
	static const std::uint16_t precondition;
	static const std::uint16_t request_too_large;
	static const std::uint16_t uri_too_long;
	static const std::uint16_t unsupported_media_type;
	static const std::uint16_t requested_range;
	static const std::uint16_t expectation_failed;
	static const std::uint16_t upgrade_required;
	static const std::uint16_t server_error;
	static const std::uint16_t not_implemented;
	static const std::uint16_t bad_gateway;
	static const std::uint16_t service_unavailable;
	static const std::uint16_t gateway_timeout;
	static const std::uint16_t not_supported;
	static const std::uint16_t authorized_cancelled;
	static const std::uint16_t pki_error;
	static const std::uint16_t webif_disabled;
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
	
	inline void
	set_content_type( const std::string &val )
	{
		m_content_type = val;
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
	
	virtual void
	add_to_header( const std::string &key, const std::string &val );
	
	inline const std::string&
	peer_host() const
	{
		return m_peer_host;
	}
	
	inline void
	set_peer_host( const std::string &val )
	{
		m_peer_host = val;
	}
	
	inline const std::string&
	peer_ethernet_addr() const
	{
		return m_peer_ethernet_addr;
	}
	
	inline void
	set_peer_ethernet_addr( const std::string &val )
	{
		m_peer_ethernet_addr = val;
	}
	
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
	
	inline bool
	secure() const
	{
		return m_secure;
	}
	
	inline void
	set_secure( bool secure )
	{
		m_secure = secure;
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
	
protected:

	void
	init();

	std::string		m_peer_host;
	std::string		m_peer_ethernet_addr;
	int				m_method;
	uri::ptr		m_uri;
	proxy::ptr		m_proxy;
	bool			m_secure;
	std::string		m_host;
	std::string		m_expect;
	std::string		m_authorization;
	std::string		m_username;
	std::string		m_password;
	std::int32_t	m_tries;
};


class response : public message
{
public:

	typedef smart_ptr< response > ptr;

	response();
	
	response( uint16_t status );

	virtual ~response();

	inline uint16_t
	status() const
	{
		return m_status;
	}

	inline void
	set_status( uint16_t status )
	{
		m_status = status;
	}
	
	virtual void
	send_prologue( connection_ptr conn ) const;

private:

	void
	init();
	
	uint16_t m_status;
};


typedef std::function< void ( http::response::ptr response, bool upgrade, bool close ) >								response_f;
typedef std::function< http::request::ptr ( int method, const uri::ptr &uri ) >											request_will_begin_f;
typedef std::function< void ( http::request::ptr request, const std::uint8_t *buf, size_t len, response_f response ) >	request_body_was_received_f;
typedef std::function< void ( http::request::ptr request, response_f func ) >											request_f;

	
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
	
	message::header				m_header;
	std::string					m_content_type;
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

inline connection&
flush( connection &conn )
{
	conn.flush();
	return conn;
}

}

}

#endif
