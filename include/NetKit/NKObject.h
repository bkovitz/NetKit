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
 
#ifndef _netkit_object_h
#define _netkit_object_h

#include <NetKit/NKSmartRef.h>
#include <NetKit/NKExpected.h>
#include <NetKit/NKStackWalk.h>
#include <unordered_map>
#include <cstdint>
#include <chrono>
#include <atomic>
#include <string>
#include <list>
#include <map>

#if defined(_WIN32) && !defined(EFI32) && !defined(EFI64)
#   define NETKIT_STDCALL __stdcall
#   if defined( NETKIT_SDK_EXPORTS )
#       define NETKIT_DLL __declspec( dllexport )
#	elif defined( NETKIT_SDK_IMPORTS )
#       define NETKIT_DLL __declspec( dllimport )
#   else
#		define NETKIT_DLL
#   endif
#else
#   define NETKIT_STDCALL
#   define NETKIT_DLL
#endif

namespace netkit {

// Forward declaration value

namespace json {

class value;
typedef smart_ref< value > value_ref;

}

class object
{
public:

	typedef std::map< std::string, std::string > attrs;
	typedef smart_ref< object > ref;
	typedef std::list< ref > list;

	object();

	object( const object& that );

	object( const json::value_ref &root );

	virtual ~object() = 0;

	virtual void
	flatten( json::value_ref &root ) const;

	json::value_ref
	json() const;

	expected< std::int8_t >
	int8_for_key( const std::string &key ) const;

	expected< std::uint8_t >
	uint8_for_key( const std::string &key ) const;

	expected< std::int16_t >
	int16_for_key( const std::string &key ) const;

	expected< std::uint16_t >
	uint16_for_key( const std::string &key ) const;

	expected< std::int32_t >
	int32_for_key( const std::string &key ) const;

	expected< std::uint32_t >
	uint32_for_key( const std::string &key ) const;

	expected< std::int64_t >
	int64_for_key( const std::string &key ) const;

	expected< std::uint64_t >
	uint64_for_key( const std::string &key ) const;

	expected< std::string >
	string_for_key( const std::string &key ) const;

	void
	set_value_for_key( const std::string &key, std::int8_t val );

	void
	set_value_for_key( const std::string &key, std::uint8_t val );
	
	void
	set_value_for_key( const std::string &key, std::int16_t val );

	void
	set_value_for_key( const std::string &key, std::uint16_t val );
	
	void
	set_value_for_key( const std::string &key, std::int32_t val );

	void
	set_value_for_key( const std::string &key, std::uint32_t val );
	
	void
	set_value_for_key( const std::string &key, std::int64_t val );

	void
	set_value_for_key( const std::string &key, std::uint64_t val );
	
#if defined( __APPLE__ )
	void
	set_value_for_key( const std::string &key, std::time_t val );
#endif
	
	void
	set_value_for_key( const std::string &key, const std::string &val );
	
	void
	remove_value_for_key( const std::string &key );

	inline attrs::iterator
	attrs_begin()
	{
		return m_attrs.begin();
	}
	
	inline attrs::iterator
	attrs_end()
	{
		return m_attrs.end();
	}
	
	inline attrs::const_iterator
	attrs_begin() const
	{
		return m_attrs.begin();
	}
	
	inline attrs::const_iterator
	attrs_end() const
	{
		return m_attrs.end();
	}
	
	inline void
	retain()
	{
		m_refs++;
	}
	
	inline int
	release()
	{
		int refs = --m_refs;
	
		if ( refs == 0 )
		{
			delete this;
		}
		
		return refs;
	}

	inline int
	refs() const
	{
		return m_refs.fetch_add( 0 );
	}
	
#if defined( NETKIT_REF_COUNT_DEBUG )

	virtual bool
	ref_count_debug();

#endif

	virtual bool
	equals( const object &that ) const;

	virtual object&
	assign( const object &that );

protected:

	void
	inflate( const json::value_ref &root );

	typedef std::atomic< int >	atomic_int_t;
	attrs						m_attrs;
	mutable atomic_int_t		m_refs;
};

extern void initialize();

}

#endif
