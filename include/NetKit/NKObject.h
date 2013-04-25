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

#include <NetKit/NKSmartPtr.h>
#include <NetKit/NKExpected.h>
#include <cstdint>
#include <atomic>
#include <string>
#include <list>
#include <map>

#if defined(_WIN32) && !defined(EFI32) && !defined(EFI64)
#   define NETKIT_STDCALL __stdcall
#   if defined( NETKIT_SDK_EXPORTS )
#       define NETKIT_DLL __declspec( dllexport )
#   else
//#       define NETKIT_DLL __declspec( dllimport )
#		define NETKIT_DLL
#   endif
#else
#   define NETKIT_STDCALL
#   define NETKIT_DLL
#endif

namespace netkit {

extern void initialize();
typedef void *tag;

class object
{
public:

	typedef std::map< std::string, std::string > keyvals;
	typedef smart_ptr< object > ptr;
	typedef std::list< ptr > list;

	virtual expected< std::int32_t >
	int_for_key( const std::string &key ) const;

	virtual expected< std::string >
	string_for_key( const std::string &key ) const;

	virtual void
	set_value_for_key( const std::string &key, std::int32_t val );
	
	virtual void
	set_value_for_key( const std::string &key, const std::string &val );
	
	inline keyvals::iterator
	keyvals_begin()
	{
		return m_map.begin();
	}
	
	inline keyvals::iterator
	keyvals_end()
	{
		return m_map.end();
	}
	
	inline keyvals::const_iterator
	keyvals_begin() const
	{
		return m_map.begin();
	}
	
	inline keyvals::const_iterator
	keyvals_end() const
	{
		return m_map.end();
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
	
	virtual bool
	equals( const object &that ) const;
	
protected:

	object();

	virtual ~object() = 0;

	typedef std::atomic< int >	atomic_int_t;
	keyvals						m_map;
	mutable atomic_int_t		m_refs;
};

}

#endif
