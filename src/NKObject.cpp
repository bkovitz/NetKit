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
 
#include <NetKit/NKObject.h>
#include <NetKit/NKJSON.h>
#include <exception>
#include <sstream>
#include <assert.h>

using namespace netkit;

object::object()
:
	m_refs( 0 )
{
	static bool first = true;
	
	if ( first )
	{
		netkit::initialize();
		first = false;
	}
}


object::object( const json::value_ref &root )
{
	inflate( root );
}


object::~object()
{
}


void
object::flatten( json::value_ref &root ) const
{
	json::value::ref attrs;

	for ( auto it = attrs_begin(); it != attrs_end(); it++ )
	{
		attrs[ it->first ] = it->second;
	}

	root[ "___attrs" ]	= attrs;
}


json::value_ref
object::json() const
{
	json::value::ref root;

	flatten( root );

	return root;
}


void
object::inflate( const json::value_ref &root )
{
	json::value::ref attrs = root[ "___attrs" ];

	if ( !attrs->is_null() )
	{
		json::value::keys members = attrs->all_keys();

		for ( size_t i = 0; i < members.size(); i++ )
		{
			std::string key = members[ i ];
			std::string val = attrs[ key ]->as_string();

			netkit::object::set_value_for_key( key, val );
		}
	}
}


expected< std::uint64_t >
object::int_for_key( const std::string &key ) const
{
	auto it = m_attrs.find( key );

	if ( it != m_attrs.end() )
	{
		return ( it->second.length() > 0 ) ? std::stoull( it->second ) : 0;
	}
	else
	{
		return std::runtime_error( "key not found" );
	}
}


expected< std::string >
object::string_for_key( const std::string &key ) const
{
	auto it = m_attrs.find( key );

	if ( it != m_attrs.end() )
	{
		return it->second;
	}
	else
	{
		return std::runtime_error( "key not found" );
	}
}


void
object::set_value_for_key( const std::string &key, std::uint64_t val )
{
	m_attrs[ key ] = std::to_string( val );
}


void
object::set_value_for_key( const std::string &key, const std::string &value )
{
	m_attrs[ key ] = value;
}


bool
object::equals( const object &that ) const
{
	return ( this == &that );
}


object&
object::assign( const object &that )
{
	return *this;
}