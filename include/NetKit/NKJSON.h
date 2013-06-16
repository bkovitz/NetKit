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
/*
 * Copyright (c) 2011 Anhero Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef _netkit_json_h
#define _netkit_json_h

#include <NetKit/NKObject.h>
#include <NetKit/NKSmartRef.h>
#include <NetKit/NKError.h>
#include <NetKit/NKExpected.h>
#include <NetKit/NKSink.h>
#include <NetKit/NKSource.h>
#include <NetKit/cstring.h>
#include <streambuf>
#include <iostream>
#include <vector>
#include <string>
#include <vector>
#include <map>

class array_map;
class object_map;

namespace netkit {

namespace json {

class NETKIT_DLL value : public netkit::object
{
public:

	typedef smart_ref< value >			ref;
	typedef std::vector<std::string>	keys;

	friend std::ostream &operator<<(std::ostream &output, const value &v);

	enum class type
	{
		string,
		integer,
		real,
		object,
		array,
		boolean,
		null,
		unknown
	};
	
	enum class output_flags
	{
		none	= 0,
		pretty	= 1
	};

	static std::string
	escape_minimum_characters(const std::string &str);

	static std::string
	escape_all_characters(const std::string &str);

	static const std::string
	escape_to_unicode(char charToEscape);
	
	static value::ref
	array();
	
	static value::ref
	object();
	
	static const value::ref
	null();
	
	static expected< value::ref >
	load( const std::string& s );
	
	value();

	value( std::istream &input );

	value( const std::string &v );

	value( const char *v );
	
	value( std::int8_t v );
	
	value( std::uint8_t v );
	
	value( std::int16_t v );
	
	value( std::uint16_t v );
	
	value( std::int32_t v );
	
	value( std::uint32_t v );
	
	value( std::int64_t v );
	
	value( std::uint64_t v );
	
#if !defined( _WIN32 )
	value( std::time_t v );
#endif
	
	value( status v );

	value( double v );

	value( bool v );

	value( const value &v );
	
	value( value::type t );

	~value();
	
	bool
	equal( const value &v ) const;

	value&
	operator=(const value &src);
		
	bool
	operator==(const value &rhs) const;

	bool
	operator!=(const value &rhs) const;
		
	bool
	operator<(const value &rhs) const;
		
	bool
	operator<=(const value &rhs) const;
		
	bool
	operator>(const value &rhs) const;
		
	bool
	operator>=(const value &rhs) const;

	value::ref
	operator[]( const char *key );

	value::ref
	operator[]( const std::string &key );

	value::ref
	at_index( std::size_t index );

	const value::ref
	at_index( std::size_t index ) const;

	type
	kind() const;

	bool
	is_string() const;

	bool
	is_integer() const;
	
	bool
	is_status() const;

	bool
	is_real() const;

	bool
	is_object() const;

	bool
	is_array() const;

	bool
	is_bool() const;

	bool
	is_null() const;

	std::string
	as_string( const std::string &default_value = "" ) const;

	void
	set_string( const std::string &v );

	std::int8_t
	as_int8( std::int8_t default_value = 0 ) const;
	
	std::uint8_t
	as_uint8( std::uint8_t default_value = 0 ) const;
	
	std::int16_t
	as_int16( std::int16_t default_value = 0 ) const;
	
	std::uint16_t
	as_uint16( std::uint16_t default_value = 0 ) const;
	
	std::int32_t
	as_int32( std::int32_t default_value = 0 ) const;
	
	std::uint32_t
	as_uint32( std::uint32_t default_value = 0 ) const;
	
	std::int64_t
	as_int64( std::int64_t default_value = 0 ) const;
	
	std::uint64_t
	as_uint64( std::uint64_t default_value = 0 ) const;
	
	std::time_t
	as_time( std::time_t default_value = 0 ) const;
	
	status
	as_status( status = status::ok ) const;

	void
	set_integer( std::int64_t v );

	double
	as_real( double default_value = 0.0f ) const;

	void
	set_real( double v );

	bool
	as_bool( bool default_value = false ) const;

	void
	set_bool( bool v );

	bool
	is_member( const std::string &key ) const;
	
	keys
	all_keys() const;
	
	void
	set_object(const object_map &newobject);

	void
	set_array(const array_map &newarray);
	
	void
	set_null();
	
	bool
	append( const value::ref &v );
	
	std::size_t
	size() const;

	void
	load_from_string(const std::string &json);

	void
	load_from_stream(std::istream &input);

	void
	load_from_file(const std::string &filePath);

	void
	write_to_stream(std::ostream &output, bool indent = true, bool escapeAll = false) const;

	void
	write_to_file(const std::string &filePath, bool indent = true, bool escapeAll = false) const;
	
	std::string
	to_string( output_flags flags = output_flags::none ) const;
	
	void
	assign( const value &v );

private:

	union data
	{
		std::string		*m_string;
		std::uint64_t	*m_integer;
		double			*m_real;
		array_map		*m_array;
		object_map		*m_object;
		bool			*m_bool;

		data();

		data( std::string *newStringvalue);

		data( std::uint64_t *newIntvalue);

		data( double *newDoublevalue);

		data( object_map *newobjectvalue);

		data( array_map *newArrayvalue);

		data(bool *newBoolvalue);
	};

	static bool
	is_hex_digit(char digit);

	static bool
	is_white_space(char whiteSpace);

	static void
	read_string(std::istream &input, std::string &result);

	static void
	read_object(std::istream &input, object_map &result);

	static void
	read_array(std::istream &input, array_map &result);

	static void
	read_number(std::istream &input, value &result);

	static void
	read_to_non_white_space(std::istream &input, char &currentCharacter);
	
	void
	clear();
	
	void
	output(std::ostream &output, bool indent = true, bool escapeAll = false) const;

	type	m_kind;
	data	m_data;
};

class NETKIT_DLL escaper
{
public:

	escaper();

	std::streambuf::int_type operator()(std::streambuf &destination, std::streambuf::int_type character);

private:

	bool m_after_back_slash;
	bool m_in_string;
};


class NETKIT_DLL connection : public sink
{
public:

	typedef std::function< void ( json::value::ref reply ) >	reply_f;
	typedef smart_ref< connection >								ref;
	typedef std::list< ref >									list;

	connection();

	virtual ~connection();

	bool
	send_notification( value::ref request );
	
	bool
	send_request( value::ref request, reply_f reply );
	
	virtual bool
	process( const std::uint8_t *buf, std::size_t len );

protected:

	typedef std::map< std::uint64_t, reply_f > reply_handlers;
	
	void
	init();

	virtual bool
	send( value::ref request );
	
	bool
	validate( const value::ref &root, value::ref error );
	
	void
	shutdown();
	
	reply_handlers						m_reply_handlers;
	static std::atomic< std::int32_t >	m_id;
};


class NETKIT_DLL server
{
public:

	typedef std::function< netkit::status ( value::ref params ) >		preflight_f;
	typedef std::function< void ( value::ref result, bool close ) >		reply_f;
	typedef std::function< void ( value::ref params ) >					notification_f;
	typedef std::function< void ( value::ref params, reply_f func ) >	request_f;

	static void
	adopt( connection::ref connection );
	
	static void
	preflight( preflight_f func );
	
	static void
	bind( const std::string &method, std::size_t num_params, notification_f func );
	
	static void
	bind( const std::string &method, std::size_t num_params, request_f func );
	
	static void
	route_notification( const value::ref &request );
	
	static void
	route_request( const value::ref &request, reply_f func );
	
	static void
	reply_with_error( reply_f reply, netkit::status status, bool close );
	
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
	
private:

	friend void												netkit::initialize();
	
	typedef std::pair< std::size_t, notification_f >		notification_target;
	typedef std::map< std::string, notification_target >	notification_handlers;
	typedef std::pair< std::size_t, request_f >				request_target;
	typedef std::map< std::string, request_target >			request_handlers;
	
	static connection::list									*m_connections;
	static connection::ref									m_active_connection;

	static preflight_f										m_preflight_handler;
	static notification_handlers							*m_notification_handlers;
	static request_handlers									*m_request_handlers;
};


class NETKIT_DLL client : public object
{
public:

	typedef std::function< void ( netkit::status error_code, const std::string &error_message, json::value::ref result ) >	reply_f;
	typedef smart_ref< client >																								ref;

	client();
	
	client( const connection::ref &conn );
	
	bool
	is_open() const;
	
	void
	connect( const uri::ref &uri, source::connect_reply_f reply );
	
	void
	on_close( sink::close_f reply );

	void
	close();

protected:

	void
	process( const std::uint8_t *buf, std::size_t len );
	
	bool
	send_notification( const std::string &method, value::ref params );
	
	bool
	send_request( const std::string &method, value::ref params, reply_f reply );
	
protected:

	connection::ref m_connection;
};

std::ostream& NETKIT_DLL
operator<<(std::ostream &output, const array_map &a);

std::ostream& NETKIT_DLL
operator<<(std::ostream& output, const object_map& o);

std::ostream& NETKIT_DLL
operator<<( std::ostream &os, const std::vector< json::value::ref > &a );

std::ostream& NETKIT_DLL
operator<<( std::ostream &os, const std::map< std::string, json::value::ref > &m );

}

template <>
class smart_ref< json::value >
{
public:

	typedef smart_ref this_type;
	typedef json::value* this_type::*unspecified_bool_type;
        
	friend std::ostream &operator<<(std::ostream &output, const smart_ref &v);
	
	inline smart_ref()
	:
		m_ref( new json::value() )
	{
		m_ref->retain();
	}
    
	inline smart_ref( bool v )
	:
		m_ref( new json::value( v ) )
	{
		m_ref->retain();
	}
	
	inline smart_ref( std::int8_t v )
	:
		m_ref( new json::value( v ) )
	{
		m_ref->retain();
	}
	
	inline smart_ref( std::uint8_t v )
	:
		m_ref( new json::value( v ) )
	{
		m_ref->retain();
	}
	
	inline smart_ref( std::int16_t v )
	:
		m_ref( new json::value( v ) )
	{
		m_ref->retain();
	}
	
	inline smart_ref( std::uint16_t v )
	:
		m_ref( new json::value( v ) )
	{
		m_ref->retain();
	}
	
	inline smart_ref( std::int32_t v )
	:
		m_ref( new json::value( v ) )
	{
		m_ref->retain();
	}
	
	inline smart_ref( std::uint32_t v )
	:
		m_ref( new json::value( v ) )
	{
		m_ref->retain();
	}
	
	inline smart_ref( std::int64_t v )
	:
		m_ref( new json::value( v ) )
	{
		m_ref->retain();
	}
	
	inline smart_ref( std::uint64_t v )
	:
		m_ref( new json::value( v ) )
	{
		m_ref->retain();
	}
	
#if !defined( _WIN32 )
	inline smart_ref( std::time_t v )
	:
		m_ref( new json::value( v ) )
	{
		m_ref->retain();
	}
#endif
	
	inline smart_ref( netkit::status v )
	:
		m_ref( new json::value( v ) )
	{
		m_ref->retain();
	}
	
	inline smart_ref( double v )
	:
		m_ref( new json::value( v ) )
	{
		m_ref->retain();
	}
	
	inline smart_ref( const char *v )
	:
		m_ref( new json::value( v ) )
	{
		m_ref->retain();
	}
	
	inline smart_ref( const std::string &v )
	:
		m_ref( new json::value( v ) )
	{
		m_ref->retain();
	}
	
	inline smart_ref( json::value *ref)
	:
		m_ref( ref )
	{
		if ( m_ref )
		{
			m_ref->retain();
		}
	}

	inline smart_ref( const smart_ref<json::value> &that )
	:
		m_ref( that.m_ref )
	{
		if ( m_ref )
		{
			m_ref->retain();
		}
	}

	inline ~smart_ref()
	{
		m_ref->release();
	}
	
	json::value*
	get() const
	{
		return m_ref;
	}

	inline json::value&
	operator*()
	{
		return *m_ref;
	}
	
	inline json::value*
	operator->()
	{
		return m_ref;
	}
	
	inline const json::value*
	operator->() const
	{
		return m_ref;
	}
    
	inline smart_ref<json::value>&
	operator=( const smart_ref<json::value> &that )
	{
		m_ref->assign( *that.m_ref );
		
		return *this;
	}
	
	inline bool
	operator==( const smart_ref<json::value> &that )
	{
		fprintf( stderr, "comparing %d to %d\n", m_ref->kind(), that.m_ref->kind() );
		return ( *m_ref == *that.m_ref );
	}

	inline bool
	operator!=( const smart_ref<json::value> &that )
	{
		return ( *m_ref != *that.m_ref );
	}
	
	inline operator bool () const
	{
		return ( !m_ref->is_null() );
	}
	
	inline operator unspecified_bool_type () const
	{
		return ( m_ref->is_null() ) ? 0 : &this_type::m_ref;
	}
	
	inline bool
	operator!() const
	{
		return ( m_ref->is_null() );
	}

	template< typename T > auto
	operator[]( T index ) -> decltype( std::to_string( index ), smart_ref< json::value >() )  // uses comma operator
	{
		return m_ref->at_index( index );
	}
	
	inline const smart_ref< json::value >
	operator[]( std::size_t index ) const
	{
		return m_ref->at_index( index );
	}

	inline smart_ref< json::value >
	operator[]( const char *key )
	{
		return ( *m_ref )[ key ];
	}
	
	inline const smart_ref< json::value >
	operator[]( const char *key ) const
	{
		return ( *m_ref )[ key ];
	}
	
	inline smart_ref< json::value >
	operator[]( const std::string &key )
	{
		return ( *m_ref )[ key ];
	}
	
	inline const smart_ref< json::value >
	operator[]( const std::string &key ) const
	{
		return ( *m_ref )[ key ];
	}
	
	template< class Other >
	operator smart_ref< Other >()
	{
		smart_ref< Other > p( m_ref );
		return p;
	}
	
	template< class Other >
	operator const smart_ref< Other >() const
	{
		smart_ref< Other > p( m_ref );
		return p;
	}
	
	inline void
	swap( smart_ref &rhs )
	{
		json::value * tmp = m_ref;
		m_ref = rhs.m_ref;
		rhs.m_ref = tmp;
	}
	
private:

	json::value *m_ref;
};

template<>
inline bool
operator==( smart_ref< json::value > const &a, smart_ref< json::value > const &b )
{
	return ( a->equal( *b.get() ) );
}

template<>
inline bool
operator!=( smart_ref< json::value > const &a, smart_ref< json::value > const &b )
{
	return ( *a.get() != *b.get() );
}

template<>
inline bool
operator==( smart_ref< json::value > const &a, json::value *b )
{
	return ( *a.get() == *b );
}

template<>
inline bool
operator!=( smart_ref< json::value > const &a, json::value *b )
{
	return ( *a.get() != *b );
}

}

#endif