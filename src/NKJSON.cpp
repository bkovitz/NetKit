#include <NetKit/NKJSON.h>
#include <string>
#include <jansson.h>
#include <stdlib.h>

using namespace netkit;
using namespace netkit::json;

const int32_t	value::flatten_flag_compact			= JSON_COMPACT;
const int32_t	value::flatten_flag_ensure_ascii	= JSON_ENSURE_ASCII;
const int32_t	value::flatten_flag_sort_keys		= JSON_SORT_KEYS;
const int32_t	value::flatten_flag_preserve_order	= JSON_PRESERVE_ORDER;
const int32_t	value::flatten_flag_encode_any		= JSON_ENCODE_ANY;
const int32_t	value::flatten_flag_escape_slash	= JSON_ESCAPE_SLASH;


expected< value::ptr >
value::load( const char *input )
{
	json_t			*impl;
	json_error_t	error;
	
	if ( ( impl = json_loads( input, 0, &error ) ) != NULL )
	{
		return value::ptr( new value( impl ) );
	}
	else
	{
		return std::logic_error( "unable to load string" );
	}
}


value::ptr
value::null()
{
	static value::ptr ret = new value( json_null() );
	
	return ret;
}

	
value::ptr
value::boolean( bool val )
{
	value::ptr ret;
	
	if ( ret )
	{
		ret = new value( json_true() );
	}
	else
	{
		ret = new value( json_false() );
	}
	
	return ret;
}

	
value::ptr
value::integer()
{
	return new value( json_integer( 0 ) );
}

	
value::ptr
value::integer( int32_t val )
{
	return new value( json_integer( val ) );
}

	
value::ptr
value::integer( uint32_t val )
{
	return new value( json_integer( val ) );
}

	
value::ptr
value::integer( int64_t val )
{
	return new value( json_integer( val ) );
}

	
value::ptr
value::integer( uint64_t val )
{
	return new value( json_integer( val ) );
}

	
value::ptr
value::real( float val )
{
	return new value( json_real( val ) );
}

	
value::ptr
value::real( double val )
{
	return new value( json_real( val ) );
}

	
value::ptr
value::string()
{
	return new value( json_string( NULL ) );
}

	
value::ptr
value::string( const char *val )
{
	return new value( json_string( val ) );
}

	
value::ptr
value::string( const std::string &val )
{
	return new value( json_string( val.c_str() ) );
}

	
value::ptr
value::array()
{
	return new value( json_array() );
}


value::ptr
value::object()
{
	return new value( json_object() );
}


value::value( json_t *impl )
:
	m_impl( impl )
{
}


value::value( const value &that )
:
	m_impl( that.m_impl )
{
	json_incref( m_impl );
}

	
value::~value()
{
	fprintf( stderr, "in value::~value( this = 0x%lx, impl = 0x%lx, impl refs = %ld )\n", this, m_impl, m_impl->refcount );
	//json_decref( m_impl );
	fprintf( stderr, "finished with decref\n" );
}

	
expected< const char* >
value::as_cstring() const
{
	if ( json_is_string( m_impl ) )
	{
		return json_string_value( m_impl );
	}
	else
	{
		return std::logic_error( "value is not a string" );
	}
}

	
expected< std::string >
value::as_string() const
{
	if ( json_is_string( m_impl ) )
	{
		return std::string( json_string_value( m_impl ) );
	}
	else
	{
		return std::logic_error( "value is not a string" );
	}
}

	
expected< int32_t >
value::as_int() const
{
	if ( json_is_integer( m_impl ) )
	{
		return static_cast< int32_t >( json_integer_value( m_impl ) );
	}
	else
	{
		return std::logic_error( "value is not a string" );
	}
}

	
expected< uint32_t >
value::as_uint() const
{
	if ( json_is_integer( m_impl ) )
	{
		return static_cast< uint32_t >( json_integer_value( m_impl ) );
	}
	else
	{
		return std::logic_error( "value is not a string" );
	}
}

	
expected< int64_t >
value::as_int64() const
{
	if ( json_is_integer( m_impl ) )
	{
		return static_cast< int64_t >( json_integer_value( m_impl ) );
	}
	else
	{
		return std::logic_error( "value is not a string" );
	}
}

	
expected< uint64_t >
value::as_uint64() const
{
	if ( json_is_integer( m_impl ) )
	{
		return static_cast< uint64_t >( json_integer_value( m_impl ) );
	}
	else
	{
		return std::logic_error( "value is not a string" );
	}
}

	
expected< float >
value::as_float() const
{
	if ( json_is_real( m_impl ) )
	{
		return static_cast< float >( json_integer_value( m_impl ) );
	}
	else
	{
		return std::logic_error( "value is not a string" );
	}
}

	
expected< double >
value::as_double() const
{
	if ( json_is_real( m_impl ) )
	{
		return json_real_value( m_impl );
	}
	else
	{
		return std::logic_error( "value is not a string" );
	}
}

	
expected< bool >
value::as_bool() const
{
	if ( json_is_true( m_impl ) )
	{
		return true;
	}
	else if ( json_is_false( m_impl ) )
	{
		return false;
	}
	else
	{
		return std::logic_error( "value is not a string" );
	}
}


bool
value::is_null() const
{
	return json_is_null( m_impl );
}

	
bool
value::is_bool() const
{
	return json_is_boolean( m_impl );
}

	
bool
value::is_integer() const
{
	return json_is_integer( m_impl );
}

	
bool
value::is_real() const
{
	return json_is_real( m_impl );
}

	
bool
value::is_numeric() const
{
	return json_is_integer( m_impl) || json_is_real( m_impl );
}


bool
value::is_string() const
{
	return json_is_string( m_impl );
}

	
bool
value::is_array() const
{
	return json_is_array( m_impl );
}

	
bool
value::is_object() const
{
	return json_is_object( m_impl );
}


size_t
value::size() const
{
	size_t s = 0;
	
	if ( json_is_array( m_impl ) )
	{
		s = json_array_size( m_impl );
	}
	
	return s;
}


bool
value::empty() const
{
	bool empty = false;
	
	if ( json_is_array( m_impl ) )
	{
		empty = json_array_size( m_impl ) == 0;
	}
	
	return empty;
}


bool
value::operator!() const
{
	return ( json_is_null( m_impl ) ) ? false : true;
}


void
value::clear()
{
	if ( json_is_object( m_impl ) )
	{
		json_object_clear( m_impl );
	}
}


value::ptr
value::operator[]( size_t index )
{
	value::ptr ret = value::null();
	
	if ( json_is_array( m_impl ) )
	{
		json_t *impl = json_array_get( m_impl, index );
		
		if ( impl )
		{
			ret = new value( impl );
		}
	}
	
	return ret;
}

	
value::ptr
value::operator[]( int index )
{
	value::ptr ret = value::null();
	
	if ( json_is_array( m_impl ) )
	{
		json_t *impl = json_array_get( m_impl, index );
		
		if ( impl )
		{
			ret = new value( impl );
		}
	}
	
	return ret;
}


const value::ptr
value::operator[]( size_t index ) const
{
	value::ptr ret = value::null();
	
	if ( json_is_array( m_impl ) )
	{
		json_t *impl = json_array_get( m_impl, index );
		
		if ( impl )
		{
			ret = new value( impl );
		}
	}
	
	return ret;
}


const value::ptr
value::operator[]( int index ) const
{
	value::ptr ret = value::null();
	
	if ( json_is_array( m_impl ) )
	{
		json_t *impl = json_array_get( m_impl, index );
		
		if ( impl )
		{
			ret = new value( impl );
		}
	}
	
	return ret;
}


bool
value::set( const char *key, const value::ptr &val )
{
	bool ok = false;
	
	if ( json_is_object( m_impl ) )
	{
		if ( json_object_set( m_impl, key, val->m_impl ) == 0 )
		{
			ok = true;
		}
	}
		
	return ok;
}


value::ptr
value::get( size_t index, const value::ptr &defaultValue ) const
{
	value::ptr ret = defaultValue;
	
	if ( json_is_array( m_impl ) )
	{
		json_t *impl = json_array_get( m_impl, index );
		
		if ( impl )
		{
			ret = value::ptr( new value( impl ) );
		}
	}
	
	return ret;
}

	
bool
value::is_valid_index( size_t index ) const
{
	bool valid = false;
	
	if ( json_is_array( m_impl ) && ( index < json_array_size( m_impl ) ) )
	{
		valid = true;
	}
	
	return valid;
}

	
bool
value::append( const value::ptr &val )
{
	bool ok = false;
	
	if ( json_is_array( m_impl ) )
	{
		if ( json_array_append( m_impl, val->m_impl ) == 0 )
		{
			ok = true;
		}
	}
	
	return ok;
}


value::ptr
value::operator[]( const char *key )
{
	value::ptr ret = value::null();
	
	if ( json_is_object( m_impl ) )
	{
		json_t *impl = json_object_get( m_impl, key );
		
		if ( impl )
		{
			ret = new value( impl );
		}
	}
	
	return ret;
}

	
const value::ptr
value::operator[]( const char *key ) const
{
	value::ptr ret = value::null();
	
	if ( json_is_object( m_impl ) )
	{
		json_t *impl = json_object_get( m_impl, key );
		
		if ( impl )
		{
			ret = new value( impl );
		}
	}
	
	return ret;
}

	
value::ptr
value::operator[]( const std::string &key )
{
	value::ptr ret = value::null();
	
	if ( json_is_object( m_impl ) )
	{
		json_t *impl = json_object_get( m_impl, key.c_str() );
		
		if ( impl )
		{
			ret = new value( impl );
		}
	}
	
	return ret;
}

	
const value::ptr
value::operator[]( const std::string &key ) const
{
	value::ptr ret = value::null();
	
	if ( json_is_object( m_impl ) )
	{
		json_t *impl = json_object_get( m_impl, key.c_str() );
		
		if ( impl )
		{
			ret = new value( impl );
		}
	}
	
	return ret;
}

	
value::ptr
value::get( const char *key, const value::ptr &defaultValue ) const
{
	value::ptr ret = defaultValue;
	
	if ( json_is_object( m_impl ) )
	{
		json_t *impl = json_object_get( m_impl, key );
		
		if ( impl )
		{
			ret = new value( impl );
		}
	}
	
	return ret;
}

	
value::ptr
value::get( const std::string &key, const value::ptr &defaultValue ) const
{
	value::ptr ret = defaultValue;
	
	if ( json_is_object( m_impl ) )
	{
		json_t *impl = json_object_get( m_impl, key.c_str() );
		
		if ( impl )
		{
			ret = new value( impl );
		}
	}
	
	return ret;
}

	
bool
value::remove_member( const char* key )
{
	bool ok = false;
	
	if ( json_is_object( m_impl ) )
	{
		if ( json_object_del( m_impl, key ) == 0 )
		{
			ok = true;
		}
	}
	
	return ok;
}


bool
value::remove_member( const std::string &key )
{
	return remove_member( key.c_str() );
}


bool
value::is_member( const char *key ) const
{
	bool ret = false;
	
	if ( json_is_object( m_impl ) )
	{
		ret = json_object_get( m_impl, key ) ? true : false;
	}
	
	return ret;
}

	
bool
value::is_member( const std::string &key ) const
{
	bool ret = false;
	
	if ( json_is_object( m_impl ) )
	{
		ret = json_object_get( m_impl, key.c_str() ) ? true : false;
	}
	
	return ret;
}


std::string
value::flatten( int flags ) const
{
	char		*str = json_dumps( m_impl, flags );
	std::string ret;
	
	if ( str )
	{
		ret = str;
		free( str );
	}
	
	return ret;
}