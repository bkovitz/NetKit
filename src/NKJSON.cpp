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


value::value( json_t *impl, bool strong_ref )
:
	m_impl( impl )
{
	if ( !strong_ref )
	{
		json_incref( m_impl );
	}
}


value::~value()
{
	json_decref( m_impl );
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
			ret = new value( impl, false );
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
			ret = new value( impl, false );
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
			ret = new value( impl, false );
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
			ret = new value( impl, false );
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
			ret = new value( impl, false );
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
			ret = new value( impl, false );
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
			ret = new value( impl, false );
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
			ret = new value( impl, false );
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
			ret = new value( impl, false );
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
			ret = new value( impl, false );
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
			ret = new value( impl, false );
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