#ifndef _CoreNetwork_CNJSON_h
#define _CoreNetwork_CNJSON_h

#include <CoreNetwork/CNObject.h>
#include <CoreNetwork/CNExpected.h>

namespace netkit {

namespace json {

class value : public object
{
public:

	typedef smart_ptr< value > ptr;

	static const uint8_t object_type;
	
	static const uint8_t array_type;
	
	static const uint8_t string_type;
	
	static const uint8_t integer_type;
	
	static const uint8_t real_type;
	
	static const uint8_t bool_type;
	
	static const uint8_t null_type;
	
	value( uint8_t type = null_type );
	
	value( int32_t value );
	
	value( uint32_t value );
	
	value( int64_t value );
	
	value( uint64_t value );
	
	value( double value );
	
    value( const char *value );
	
	value( const char *beginValue, const char *endValue );
	
	value( const std::string &value );
	
	value( bool value );
	
	value( const value &other );
	
	virtual ~value();
	
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
	is_null() const;
	
	bool
	is_bool() const;
	
	bool
	is_int() const;
	
	bool
	is_uint() const;
	
	bool
	is_integral() const;
	
	bool
	is_double() const;
	
	bool
	is_numeric() const;
	
	bool
	is_string() const;
	
	bool
	is_array() const;
	
	bool
	is_object() const;

	bool
	is_convertible_to( uint8_t other ) const;

    size_t
	size() const;

	bool
	empty() const;

	bool
	operator!() const;

	void
	clear();

	void
	resize( size_t size );

	expected< value::ptr >
	operator[]( size_t index );
	
	expected< value::ptr >
	operator[]( int index );

	expected< const value::ptr >
	operator[]( size_t index ) const;

	expected< const value::ptr >
	operator[]( int index ) const;

	expected< value::ptr >
	get( size_t index, const value::ptr &defaultValue ) const;
	
	bool
	is_valid_index( size_t index ) const;
	
	value::ptr
	append( const value::ptr &value );

	value::ptr
	operator[]( const char *key );
	
	const value::ptr
	operator[]( const char *key ) const;
	
	value::ptr
	operator[]( const std::string &key );
	
	const value::ptr
	operator[]( const std::string &key ) const;
	
	value::ptr
	get( const char *key, const value::ptr &defaultValue ) const;
	
	value::ptr
	get( const std::string &key, const value::ptr &defaultValue ) const;
	
	value::ptr
	remove_member( const char* key );
	
	value::ptr
	remove_member( const std::string &key );

	bool
	is_member( const char *key ) const;
	
	bool
	is_member( const std::string &key ) const;
	
private:

	struct json_t *m_impl;
};

}

}

#endif
