#ifndef _netkit_json_h
#define _netkit_json_h

#include <NetKit/NKSink.h>
#include <NetKit/NKSource.h>
#include <NetKit/NKExpected.h>
#include <map>

struct json_t;

namespace netkit {

namespace json {

class value : public object
{
public:

	static const int32_t	flatten_flag_compact;
	static const int32_t	flatten_flag_ensure_ascii;
	static const int32_t	flatten_flag_sort_keys;
	static const int32_t	flatten_flag_preserve_order;
	static const int32_t	flatten_flag_encode_any;
	static const int32_t	flatten_flag_escape_slash;

public:

	typedef smart_ptr< value > ptr;
	
	static expected< value::ptr >
	load( const char *input );
	
	static value::ptr
	null();
	
	static value::ptr
	boolean( bool val );
	
	static value::ptr
	integer();
	
	static value::ptr
	integer( int32_t value );
	
	static value::ptr
	integer( uint32_t value );
	
	static value::ptr
	integer( int64_t value );
	
	static value::ptr
	integer( uint64_t value );
	
	static value::ptr
	real( float val );
	
	static value::ptr
	real( double val );
	
	static value::ptr
	string();
	
	static value::ptr
	string( const char *val );
	
	static value::ptr
	string( const std::string &val );
	
	static value::ptr
	array();
	
	static value::ptr
	object();
	
	bool
	is_null() const;
	
	bool
	is_bool() const;
	
	bool
	is_integer() const;
	
	bool
	is_real() const;
	
	bool
	is_numeric() const;
	
	bool
	is_string() const;
	
	bool
	is_array() const;
	
	bool
	is_object() const;
	
	expected< const char* >
	as_cstring() const;
	
	expected< std::string >
	as_string() const;
	
	expected< int32_t >
	as_int() const;
	
	expected< uint32_t >
	as_uint() const;
	
	expected< int64_t >
	as_int64() const;
	
	expected< uint64_t >
	as_uint64() const;
	
	expected< float >
	as_float() const;
	
	expected< double >
	as_double() const;
	
	expected< bool >
	as_bool() const;

	bool
	empty() const;

	bool
	operator!() const;

	void
	clear();

	size_t
	size() const;

	void
	resize( size_t size );

	value::ptr
	operator[]( size_t index );
	
	value::ptr
	operator[]( int index );

	const value::ptr
	operator[]( size_t index ) const;

	const value::ptr
	operator[]( int index ) const;

	value::ptr
	get( size_t index, const value::ptr &defaultValue ) const;
	
	bool
	is_valid_index( size_t index ) const;
	
	bool
	append( const value::ptr &value );

	value::ptr
	operator[]( const char *key );
	
	const value::ptr
	operator[]( const char *key ) const;
	
	value::ptr
	operator[]( const std::string &key );
	
	const value::ptr
	operator[]( const std::string &key ) const;
	
	bool
	set( const char *key, const value::ptr &val );
	
	value::ptr
	get( const char *key, const value::ptr &defaultValue ) const;
	
	value::ptr
	get( const std::string &key, const value::ptr &defaultValue ) const;
	
	bool
	remove_member( const char* key );
	
	bool
	remove_member( const std::string &key );

	bool
	is_member( const char *key ) const;
	
	bool
	is_member( const std::string &key ) const;
	
	std::string
	flatten( int32_t flags ) const;
	
protected:

	value( json_t *impl );
	
	value( const value &that );
	
	virtual ~value();
	
private:

	json_t *m_impl;
};


class message : public object
{
public:

	typedef smart_ptr< message > ptr;

	message();
	
	virtual ~message();

	std::string
	version();
	
	value::ptr
	id();
	
protected:

	value::ptr m_root;
};


class request : public message
{
public:

	typedef smart_ptr< request > ptr;
	
	request();
	
	virtual ~request();

	std::string
	method();
	
	value::ptr
	params();
};


class response : public message
{
public:

	typedef smart_ptr< response > ptr;
	
	value::ptr
	result();
	
	value::ptr
	error();
};


typedef std::function< void ( response::ptr response ) >				response_f;
typedef std::function< void ( request::ptr request, response_f func ) >	request_f;

	
class connection : public sink
{
public:

	static sink::ptr
	adopt( const std::uint8_t *buf, size_t len );
	
	static void
	bind( const std::string &method, request_f func );
	
	virtual ssize_t
	read( source::ptr s );
	
private:

	typedef std::map< std::string, request_f > bindings;
	
	static bindings m_bindings;
};

}

}

#endif
