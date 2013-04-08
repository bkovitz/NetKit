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
#include <NetKit/NKSmartPtr.h>
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

class value : public netkit::object
{
public:

	typedef smart_ptr< value >			ptr;
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
	
	static value::ptr
	array();
	
	static value::ptr
	object();
	
	static const value::ptr
	null();
	
	static expected< value::ptr >
	load( const std::string& s );
	
	value();

	value(std::istream &input);

	value( const std::string &v );

	value( const char *v );

	value( int v);
	
	value( status v );

	value( double v );

	value( bool v );

	value( const value &v );
	
	value( enum type t );

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

	value::ptr
	operator[](const char *key);

	value::ptr
	operator[](const std::string &key);

	value::ptr
	operator[](size_t index);

	type
	type() const;

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

	int
	as_integer( int default_value = 0 ) const;
	
	status
	as_status( status = status::ok ) const;

	void
	set_integer( int v );

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
	append( const value::ptr &v );
	
	size_t
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
	flatten( output_flags flags = output_flags::none ) const;
	
	void
	assign( const value &v );

private:

	union data
	{
		std::string		*m_string;
		int				*m_integer;
		double			*m_real;
		array_map		*m_array;
		object_map		*m_object;
		bool			*m_bool;

		data();

		data( std::string *newStringvalue);

		data( int *newIntvalue);

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

	enum type	m_type;
	data		m_data;
};

class escaper
{
public:

	escaper();

	std::streambuf::int_type operator()(std::streambuf &destination, std::streambuf::int_type character);

private:

	bool m_after_back_slash;
	bool m_in_string;
};


class connection : public sink
{
public:

	typedef std::function< void ( json::value::ptr reply ) >	reply_f;
	typedef smart_ptr< connection >								ptr;
	typedef std::list< ptr >									list;
	
	connection( const source::ptr &source );
	
	virtual ~connection();

	static sink::ptr
	adopt( source::ptr source, const std::uint8_t *buf, size_t len );
	
	inline static connection::ptr
	active()
	{
		return m_active;
	}
	
	inline static list::iterator
	begin()
	{
		return m_instances->begin();
	}
	
	inline static list::iterator
	end()
	{
		return m_instances->end();
	}
	
	bool
	send_notification( value::ptr request );
	
	bool
	send_request( value::ptr request, reply_f reply );
	
	virtual ssize_t
	process();
	
protected:

	typedef std::map< std::int32_t, reply_f > reply_handlers;
	
	virtual bool
	send( value::ptr request );
	
	inline size_t
    num_bytes_used()
    {
		return m_eptr - m_base;
    }
	
	inline size_t
	num_bytes_unused()
	{
		return m_end - m_eptr;
	}

    inline size_t
    size()
    {
        return m_end - m_base;
    }

    inline void
    add( size_t numBytes )
    {
        size_t bytesUsed = num_bytes_used();
        size_t oldSize = size();
        size_t newSize = oldSize + numBytes;

        m_base = ( unsigned char* ) realloc( m_base, newSize );

        m_eptr = m_base + bytesUsed;
        m_end = m_base + newSize;
    }

    inline void
    shift( size_t index )
    {
        if ( ( m_base + index ) < m_eptr )
        {
            int delta = ( int ) ( ( m_eptr - ( m_base + index ) ) );
            std::memmove_s( m_base, size(), m_base + index, delta );
            m_eptr = m_base + delta;
        }
        else
        {
            m_eptr = m_base;
        }
    }
	
	std::string
	encode( const std::string &s );
	
	bool
	really_process();
	
	bool
	validate( const value::ptr &root, value::ptr error );
	
	void
	shutdown();
	
	friend void							netkit::initialize();
		
	static connection::list				*m_instances;
	static connection::ptr				m_active;
	reply_handlers						m_reply_handlers;
	static std::atomic< std::int32_t >	m_id;
	
	std::string							m_token;
	std::uint8_t						*m_base;
	std::uint8_t						*m_eptr;
	std::uint8_t						*m_end;
};


class server
{
public:

	typedef std::function< netkit::status ( value::ptr params ) >					preflight_f;
	typedef std::function< void ( value::ptr result, bool upgrade, bool close ) >	reply_f;
	typedef std::function< void ( value::ptr params ) >								notification_f;
	typedef std::function< void ( value::ptr params, reply_f func ) >				request_f;
	
	static void
	preflight( preflight_f func );
	
	static void
	bind( const std::string &method, size_t num_params, notification_f func );
	
	static void
	bind( const std::string &method, size_t num_params, request_f func );
	
	static void
	route_notification( const value::ptr &request );
	
	static void
	route_request( const value::ptr &request, reply_f func );
	
	static void
	reply_with_error( reply_f reply, netkit::status status, bool upgrade, bool close );
	
private:

	friend void												netkit::initialize();
	
	typedef std::pair< size_t, notification_f >				notification_target;
	typedef std::map< std::string, notification_target >	notification_handlers;
	typedef std::pair< size_t, request_f >					request_target;
	typedef std::map< std::string, request_target >			request_handlers;
	
	static preflight_f										m_preflight_handler;
	static notification_handlers							*m_notification_handlers;
	static request_handlers									*m_request_handlers;
};


class client : public object
{
public:

	typedef std::function< void ( netkit::status error_code, const std::string &error_message, json::value::ptr result ) >	reply_f;
	typedef smart_ptr< client >																								ptr;

	client( const source::ptr &source );
	
	client( const connection::ptr &conn );
	
	ssize_t
	process();
	
	bool
	is_open() const;
	
	void
	close();
	
protected:

	bool
	send_notification( const std::string &method, value::ptr params );
	
	bool
	send_request( const std::string &method, value::ptr params, reply_f reply );
	
protected:

	connection::ptr m_connection;
};

std::ostream&
operator<<(std::ostream &output, const array_map &a);

std::ostream&
operator<<(std::ostream& output, const object_map& o);

std::ostream&
operator<<( std::ostream &os, const std::vector< json::value::ptr > &a );

std::ostream&
operator<<( std::ostream &os, const std::map< std::string, json::value::ptr > &m );

}

template <>
class smart_ptr< json::value >
{
public:

	typedef smart_ptr this_type;
	typedef json::value* this_type::*unspecified_bool_type;
        
	friend std::ostream &operator<<(std::ostream &output, const smart_ptr &v);
	
	inline smart_ptr()
	:
		m_ref( new json::value() )
	{
		m_ref->retain();
	}
    
	inline smart_ptr( bool v )
	:
		m_ref( new json::value( v ) )
	{
		m_ref->retain();
	}
	
	inline smart_ptr( int32_t v )
	:
		m_ref( new json::value( v ) )
	{
		m_ref->retain();
	}
	
	inline smart_ptr( netkit::status v )
	:
		m_ref( new json::value( v ) )
	{
		m_ref->retain();
	}
	
	inline smart_ptr( double v )
	:
		m_ref( new json::value( v ) )
	{
		m_ref->retain();
	}
	
	inline smart_ptr( const char *v )
	:
		m_ref( new json::value( v ) )
	{
		m_ref->retain();
	}
	
	inline smart_ptr( const std::string &v )
	:
		m_ref( new json::value( v ) )
	{
		m_ref->retain();
	}
	
	inline smart_ptr( json::value *ref)
	:
		m_ref( ref )
	{
		m_ref->retain();
	}

	inline smart_ptr( const smart_ptr<json::value> &that )
	:
		m_ref( that.m_ref )
	{
		m_ref->retain();
	}

	inline ~smart_ptr()
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
    
	inline smart_ptr<json::value>&
	operator=( const smart_ptr<json::value> &that )
	{
		m_ref->assign( *that.m_ref );
		
		return *this;
	}
	
	inline bool
	operator==( const smart_ptr<json::value> &that )
	{
		fprintf( stderr, "comparing %d to %d\n", m_ref->type(), that.m_ref->type() );
		return ( *m_ref == *that.m_ref );
	}

	inline bool
	operator!=( const smart_ptr<json::value> &that )
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
	
	inline smart_ptr< json::value >
	operator[]( size_t index )
	{
		return ( *m_ref )[ index ];
	}
	
	inline const smart_ptr< json::value >
	operator[]( size_t index ) const
	{
		return ( *m_ref )[ index ];
	}

	inline smart_ptr< json::value >
	operator[]( const char *key )
	{
		return ( *m_ref )[ key ];
	}
	
	inline const smart_ptr< json::value >
	operator[]( const char *key ) const
	{
		return ( *m_ref )[ key ];
	}
	
	inline smart_ptr< json::value >
	operator[]( const std::string &key )
	{
		return ( *m_ref )[ key ];
	}
	
	inline const smart_ptr< json::value >
	operator[]( const std::string &key ) const
	{
		return ( *m_ref )[ key ];
	}
	
	template< class Other >
	operator smart_ptr< Other >()
	{
		smart_ptr< Other > p( m_ref );
		return p;
	}
	
	template< class Other >
	operator const smart_ptr< Other >() const
	{
		smart_ptr< Other > p( m_ref );
		return p;
	}
	
	inline void
	swap( smart_ptr &rhs )
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
operator==( smart_ptr< json::value > const &a, smart_ptr< json::value > const &b )
{
	return ( a->equal( *b.get() ) );
}

template<>
inline bool
operator!=( smart_ptr< json::value > const &a, smart_ptr< json::value > const &b )
{
	return ( *a.get() != *b.get() );
}

template<>
inline bool
operator==( smart_ptr< json::value > const &a, json::value *b )
{
	return ( *a.get() == *b );
}

template<>
inline bool
operator!=( smart_ptr< json::value > const &a, json::value *b )
{
	return ( *a.get() != *b );
}

}

#endif