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
 
#ifndef _netkit_smart_ref_h
#define _netkit_smart_ref_h

#include <unordered_map>
#include <functional>
#include <assert.h>
#include <stdio.h>
#include <typeinfo>

// #define NETKIT_REF_COUNT_DEBUG

namespace netkit {

#if defined( NETKIT_REF_COUNT_DEBUG )

extern std::unordered_map< void*, std::string > ref_count_map;
extern void ref_count_print( const std::string &message );

#endif

template < class T >
class smart_ref
{
public:

	typedef smart_ref this_type;
	typedef T* this_type::*unspecified_bool_type;
        
	inline smart_ref()
	:
		m_ref( NULL )
	{
	}
    
	inline smart_ref( T *ref)
	:
		m_ref( ref )
	{
		if ( m_ref )
		{
#if defined( NETKIT_REF_COUNT_DEBUG )
			if ( m_ref->ref_count_debug() )
			{
				ref_count_map[ this ] = netkit::stackwalk::copy();
			}
#endif
			m_ref->retain();
		}
	}

	inline smart_ref( const smart_ref<T> &that )
	:
		m_ref( that.m_ref )
	{
		if ( m_ref )
		{
#if defined( NETKIT_REF_COUNT_DEBUG )
			if ( m_ref->ref_count_debug() )
			{
				ref_count_map[ this ] = netkit::stackwalk::copy();
			}
#endif
			m_ref->retain();
		}
	}

	inline ~smart_ref()
	{
		if ( m_ref )
		{
#if defined( NETKIT_REF_COUNT_DEBUG )
			if ( m_ref->ref_count_debug() )
			{
				auto it = ref_count_map.find( this );

				if ( it != ref_count_map.end() )
				{
					ref_count_map.erase( it );
					ref_count_print( std::string( "in destructor with refs " ) + std::to_string( m_ref->refs() ) );
				}
			}
#endif

			if ( m_ref->release() == 0 )
			{
				m_ref = NULL;
			}
        }
	}
	
	T*
	get() const
	{
		return m_ref;
	}

	inline T&
	operator*()
	{
		return *m_ref;
	}
	
	
	inline const T&
	operator*() const
	{
		return *m_ref;
	}
	
    
	inline T*
	operator->()
	{
		return m_ref;
	}
	
	
	inline const T*
	operator->() const
	{
		return m_ref;
	}
	
    
	inline smart_ref<T>&
	operator=( const smart_ref<T> &that )
	{
		if ( this != &that )
		{
			if ( m_ref )
			{
#if defined( NETKIT_REF_COUNT_DEBUG )
				if ( m_ref->ref_count_debug() )
				{
					auto it = ref_count_map.find( this );
	
					if ( it != ref_count_map.end() )
					{
						ref_count_map.erase( it );
						ref_count_print( std::string( "in assignment operator with refs " ) + std::to_string( m_ref->refs() ) );
					}
				}
#endif
				m_ref->release();
				m_ref = NULL;
			}

			m_ref = that.m_ref;
			
			if ( m_ref )
			{
#if defined( NETKIT_REF_COUNT_DEBUG )
				if ( m_ref->ref_count_debug() )
				{
					ref_count_map[ this ] = netkit::stackwalk::copy();
				}
#endif
				m_ref->retain();
			}
		}
		
		return *this;
	}
	
	inline bool
	operator==( const smart_ref<T> &that )
	{
		return ( m_ref == that.m_ref );
	}

	inline operator bool () const
	{
		return ( m_ref != NULL );
	}
	
	inline operator unspecified_bool_type () const
	{
		return ( m_ref == 0 ) ? 0 : &this_type::m_ref;
	}
	
	inline bool
	operator!() const
	{
		return ( m_ref == NULL );
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
		T * tmp = m_ref;
		m_ref = rhs.m_ref;
		rhs.m_ref = tmp;
	}
	
private:

	T *m_ref;
};

template< class T, class U >
inline bool
operator==( smart_ref< T > const &a, smart_ref< U > const &b )
{
	return a.get() == b.get();
}

template< class T, class U >
inline bool
operator!=( smart_ref< T > const &a, smart_ref< U > const &b )
{
	return a.get() != b.get();
}

template< class T >
inline bool
operator==( smart_ref< T > const &a, T *b )
{
	return a.get() == b;
}

template< class T >
inline bool
operator!=( smart_ref< T > const &a, T *b )
{
	return a.get() != b;
}

template< class T >
inline bool
operator==( T *a, smart_ref< T > const &b )
{
	return a == b.get();
}

template< class T >
inline bool
operator!=( T *a, smart_ref< T > const &b )
{
	return a != b.get();
}

template< class T >
inline bool
operator<( smart_ref< T > const &a, smart_ref< T > const &b )
{
	return std::less< T* >()( a.get(), b.get() );
}

template< class T >
void
swap( smart_ref< T > &lhs, smart_ref< T > &rhs )
{
	lhs.swap( rhs );
}

template< class T >
T*
get_pointer( smart_ref< T > const &p )
{
	return p.get();
}

template <class T, class U >
smart_ref< T >
static_pointer_cast( smart_ref< U > const &p )
{
	return static_cast< T* >( p.get() );
}

template< class T, class U >
smart_ref< T >
const_pointer_cast( smart_ref< U > const &p )
{
	return const_cast< T* >( p.get() );
}

template< class T, class U >
smart_ref< T >
dynamic_pointer_cast( smart_ref< U > const &p )
{
	return dynamic_cast< T* >( p.get() );
}

}

#endif