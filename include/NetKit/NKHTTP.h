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

#include <NetKit/NKSource.h>
#include <NetKit/NKProxy.h>
#include <NetKit/NKURI.h>
#include <NetKit/NKString.h>
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
typedef smart_ref< connection > connection_ref;

struct method
{
	static std::string NETKIT_DLL
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
	static std::string NETKIT_DLL
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


class NETKIT_DLL message : public object
{
public:

	typedef std::map< std::string, std::string >	header;
	typedef smart_ref< message >					ref;
	
public:

	message( std::uint16_t major, std::uint16_t minor );
	
	message( const message &that );
	
	virtual ~message();
	
	inline std::uint16_t
	major() const
	{
		return m_major;
	}
	
	inline std::uint16_t
	minor() const
	{
		return m_minor;
	}
	
	inline const header&
	heeder() const
	{
		return m_header;
	}
	
	virtual void
	add_to_header( const header& header );

	virtual void
	add_to_header( const std::string &key, int val );

	virtual void
	add_to_header( const std::string &key, const std::string &val );

	virtual std::string
	find_in_header( const std::string &key ) const;

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

	inline const std::string&
	upgrade() const
	{
		return m_upgrade;
	}

	inline const std::string&
	ws_key() const
	{
		return m_ws_key;
	}

	inline bool
	keep_alive() const
	{
		return m_keep_alive;
	}

	inline void
	set_keep_alive( bool val )
	{
		m_keep_alive = val;
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
	preflight();
	
	virtual void
	send_prologue( connection_ref conn ) const = 0;
	
	virtual bool
	send_body( connection_ref conn ) const;
	
protected:

	std::uint16_t		m_major;
	std::uint16_t		m_minor;
	header				m_header;
	std::string			m_content_type;
	size_t				m_content_length;
	std::string			m_upgrade;
	std::string			m_ws_key;
	bool				m_keep_alive;
	std::ostringstream	m_ostream;
};


class response;
typedef smart_ref< response > response_ref;

class NETKIT_DLL request : public message
{
public:

	typedef std::function< bool ( uint32_t status ) >													auth_f;
	typedef std::function< void ( response_ref response ) >												headers_reply_f;
	typedef std::function< void ( response_ref response, const std::uint8_t *buf, std::size_t len ) >	body_reply_f;
	typedef std::function< void ( response_ref response ) >												reply_f;
	typedef smart_ref< request >																		ref;

	request( int method, std::uint16_t major, std::uint16_t minor, const uri::ref &uri );

	virtual ~request();

	virtual void
	add_to_header( const header& header );

	virtual void
	add_to_header( const std::string &key, const std::string &val );

	template< class T > auto
	add_to_header( const std::string &key, const T &val ) -> decltype( std::to_string( val ), void() )
	{
		add_to_header( key, std::to_string( val ) );
	}

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

	inline uri::ref&
	uri() 
	{
		return m_uri;
	}
	
	inline const uri::ref&
	uri() const
	{
		return m_uri;
	}

	inline void
	set_uri( const netkit::uri::ref &val )
	{
		m_uri = val;
	}

	inline const std::string&
	expect() const
	{
		return m_expect;
	}
	
	inline const proxy::ref&
	proxy() const
	{
		return m_proxy;
	}
	
	inline void
	set_proxy( const proxy::ref &proxy )
	{
		m_proxy = proxy;
	}

	inline bool
	redirect() const
	{
		return m_redirect;
	}

	inline void
	set_redirect( bool val )
	{
		m_redirect = val;
	}

	virtual void
	send_prologue( connection_ref conn ) const;
	
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

	void
	auth( int status );

	inline void
	on_auth( auth_f auth )
	{
		m_auth = auth;
	}

	void
	headers_reply( response_ref response );

	inline void
	on_headers_reply( headers_reply_f reply )
	{
		m_headers_reply = reply;
	}

	void
	body_reply( response_ref response, const std::uint8_t *buf, std::size_t len );

	inline void
	on_body_reply( body_reply_f reply )
	{
		m_body_reply = reply;
	}

	void
	reply( response_ref response );

	inline void
	on_reply( reply_f reply )
	{
		m_reply = reply;
	}

	inline std::int32_t
	max_redirects() const
	{
		return m_max_redirects;
	}

	inline void
	set_max_redirects( std::int32_t val )
	{
		m_max_redirects = val;
	}

	inline bool
	can_redirect()
	{
		return ( m_num_redirects < m_max_redirects ) ? true : false;
	}

	inline void
	redirect()
	{
		m_num_redirects++;
	}

protected:

	request( const request &that );

	void
	init();

	std::int32_t	m_max_redirects;
	std::int32_t	m_num_redirects;
	auth_f			m_auth;
	headers_reply_f	m_headers_reply;
	body_reply_f	m_body_reply;
	reply_f			m_reply;

	std::string		m_peer_host;
	std::string		m_peer_ethernet_addr;
	int				m_method;
	uri::ref		m_uri;
	proxy::ref		m_proxy;
	bool			m_redirect;
	std::string		m_host;
	std::string		m_expect;
	std::string		m_authorization;
	std::string		m_username;
	std::string		m_password;
	std::int32_t	m_tries;
};


class NETKIT_DLL response : public message
{
public:

	typedef smart_ref< response > ref;

	response( std::uint16_t major, std::uint16_t minor, std::uint16_t status, bool keep_alive );

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
	send_prologue( connection_ref conn ) const;

protected:

	response( const response &that );

	void
	init();
	
	std::uint16_t m_status;
};


class NETKIT_DLL connection : public sink
{
public:

	typedef smart_ref< connection > ref;
	typedef std::list< ref > list;

	class handler : public object
	{
	public:

		typedef smart_ref< handler > ref;

		virtual void
		process_will_begin( connection::ref connection ) = 0;

		virtual void
		message_will_begin( connection::ref connection ) = 0;

		virtual int
		uri_was_received( connection::ref connection, const char *buf, size_t len ) = 0;

		virtual int
		headers_were_received( connection::ref connection, message::header &header ) = 0;

		virtual int
		body_was_received( connection::ref connection, const char *buf, size_t len ) = 0;

		virtual int
		message_was_received( connection::ref connection ) = 0;

		virtual void
		process_did_end( connection::ref connection ) = 0;

		virtual void
		will_close( connection::ref connection ) = 0;
	};

	connection( handler::ref handler );

	virtual ~connection();

	inline handler::ref
	handler()  const
	{
		return m_handler;
	}

	virtual bool
	process( const std::uint8_t *buf, std::size_t len );

	bool
	upgrade_to_ws( const request::ref &request, std::function< void ( http::response::ref response, bool close ) > reply );

	bool
	upgrade_to_tls( const request::ref &request, std::function< void ( http::response::ref response, bool close ) > reply );

	void
	upgrade( sink::ref sink );

	inline bool
	secure() const
	{
		return m_secure;
	}

	void
	set_secure( bool val, bool is_server = true );
	
	bool
	put( message::ref message );
	
	int
	method() const;

	int
	status_code() const;

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

	virtual void
	close();

protected:

	enum
	{
		NONE = 0,
		FIELD,
		VALUE
	};
	
protected:

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
	
	void
	init();
	
	friend class				server;
	
	std::string					m_header_field;
	std::string					m_header_value;
	time_t						m_start;
	bool						m_okay;
	std::vector< std::uint8_t >	m_body;
	
	bool						m_secure;
	http_parser_settings		*m_settings;
	http_parser					*m_parser;
	int							m_parse_state;
	
	message::header				m_header;

	std::ostringstream			m_ostream;

	handler::ref				m_handler;
};


class NETKIT_DLL server
{
public:

	typedef std::function< void ( http::response::ref response, bool close ) >													response_f;
	typedef std::function< http::request::ref ( int method, std::uint16_t major, std::uint16_t minor, const uri::ref &uri ) >	request_will_begin_f;
	typedef std::function< int ( http::request::ref request, const std::uint8_t *buf, size_t len, response_f response ) >		request_body_was_received_f;
	typedef std::function< int ( http::request::ref request, response_f func ) >												request_f;

	class binding : public netkit::object
	{
	public:

		typedef smart_ref< binding > ref;
		typedef std::list< ref > list;

		binding( const std::string &path, const std::string &type, request_f r )
		:
			m_path( path ),
			m_type( type ),
			m_r( r )
		{
			m_rwb = [=]( int method, std::uint16_t major, std::uint16_t minor, const uri::ref &uri )
			{
				return new http::request( method, major, minor, uri );
			};

			m_rbwr = [=]( http::request::ref request, const std::uint8_t *buf, size_t len, response_f response )
			{
				return 0;
			};
		}
	
		binding( const std::string &path, const std::string &type, request_body_was_received_f rbwr, request_f r )
		:
			m_path( path ),
			m_type( type ),
			m_rbwr( rbwr ),
			m_r( r )
		{
			m_rwb = [=]( int method, std::uint16_t major, std::uint16_t minor, const uri::ref &uri )
			{
				return new http::request( method, major, minor, uri );
			};
		}
	
		binding( const std::string &path, const std::string &type, request_will_begin_f rwb, request_body_was_received_f rbwr, request_f r )
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

	static sink::ref
	adopt( source::ref source );

	static void
	bind( std::uint8_t method, const std::string &path, const std::string &type, request_f r );
	
	static void
	bind( std::uint8_t method, const std::string &path, const std::string &type, request_body_was_received_f rbwr, request_f r );
	
	static void
	bind( std::uint8_t method, const std::string &path, const std::string &type, request_will_begin_f rwb, request_body_was_received_f rbwr, request_f r );
	
	static void
	bind( std::uint8_t method, const std::string &path, sink::ref sink );

	static binding::ref
	resolve( connection::ref conn, const message::header &header );

	inline static connection::ref
	active_connection()
	{
		return m_active_connection;
	}

	inline static void
	set_active_connection( connection *c )
	{
		m_active_connection = c;
	}

	inline static connection::list::iterator
	begin_connections()
	{
		return m_connections->begin();
	}
	
	inline static connection::list::iterator
	end_connections()
	{
		return m_connections->end();
	}

protected:

	class handler : public connection::handler
	{
	public:

		typedef smart_ref< handler > ref;

		handler();

		virtual ~handler();
	
		virtual void
		process_will_begin( connection::ref connection );

		virtual void
		message_will_begin( connection::ref connection );

		virtual int
		uri_was_received( connection::ref connection, const char *buf, size_t len );

		virtual int
		headers_were_received( connection::ref connection, message::header &header );

		virtual int
		body_was_received( connection::ref connection, const char *buf, size_t len );

		virtual int
		message_was_received( connection::ref connection );

		virtual void
		process_did_end( connection::ref connection );

		virtual void
		will_close( connection::ref connection );

		std::string		m_uri;
		binding::ref	m_binding;
		request::ref	m_request;
	};

	friend void					netkit::initialize();

	static void
	bind( std::uint8_t method, binding::ref binding );

	static std::string
	regexify( const std::string &s );

	static void
	replace( std::string& str, const std::string& oldStr, const std::string& newStr);
	
	typedef std::map< std::uint8_t, binding::list > bindings;

	static connection::ref		m_active_connection;

	static connection::list		*m_connections;
	static bindings				m_bindings;
};


class NETKIT_DLL client : public connection::handler
{
public:

	typedef smart_ref< client > ref;

	static request::ref
	request( int method, const uri::ref &uri );

	static void
	send( const request::ref &request );

protected:

	client( const request::ref &request );

	void
	really_send();

	virtual ~client();

	virtual void
	process_will_begin( connection::ref connection );

	virtual void
	message_will_begin( connection::ref connection );

	virtual int
	uri_was_received( connection::ref connection, const char *buf, size_t len );

	virtual int
	headers_were_received( connection::ref connection, message::header &header );

	virtual int
	body_was_received( connection::ref connection, const char *buf, size_t len );

	virtual int
	message_was_received( connection::ref connection );

	virtual void
	process_did_end( connection::ref connection );

	virtual void
	will_close( connection::ref connection );

	connection::ref	m_connection;
	request::ref	m_request;
	response::ref	m_response;
	std::string		m_redirect;
	bool			m_done;
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
