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


object::~object()
{
	//assert( m_refs == 0 );
}


expected< std::int32_t >
object::int_for_key( const std::string &key ) const
{
	auto it = m_map.find( key );

	if ( it != m_map.end() )
	{
		return ( it->second.length() > 0 ) ? atoi( it->second.c_str() ) : 0;
	}
	else
	{
		return std::runtime_error( "key not found" );
	}
}


expected< std::string >
object::string_for_key( const std::string &key ) const
{
	auto it = m_map.find( key );

	if ( it != m_map.end() )
	{
		return it->second;
	}
	else
	{
		return std::runtime_error( "key not found" );
	}
}


void
object::set_value_for_key( const std::string &key, std::int32_t val )
{
	std::ostringstream os;
	
	os << val;
	
	m_map[ key ] = os.str();
}


void
object::set_value_for_key( const std::string &key, const std::string &value )
{
	m_map[ key ] = value;
}