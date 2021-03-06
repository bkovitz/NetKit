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
 
#ifndef _netkit_Expected_h
#define _netkit_Expected_h

//! compile time max(A, B)
template <unsigned A, unsigned B>
struct max_of
{
    static unsigned const value = B < A ? A : B;
};

namespace netkit {

template <typename T>
class expected
{
public:

	expected()
	:
		m_expected( false )
	{
		new( &m_storage ) std::runtime_error( "" );
	}

    expected( expected const& that )
	:
		m_expected( that.m_expected )
	{
		if ( that.is_valid() )
		{
            new( &m_storage ) T( that.as_value());
        }
		else
		{
			new( &m_storage ) std::exception_ptr( that.as_exception() );
        }
    }
	
	expected( expected&& that )
	:
		m_expected( that.m_expected )
	{
		if ( that.is_valid() )
		{
			new( &m_storage ) T( std::move( that.as_value() ) );
		}
		else
		{
			new( &m_storage ) std::exception_ptr( std::move( that.as_exception() ) );
        }
    }

	inline expected&
	operator=( expected const& that )
	{
		~expected();

		m_expected = that.m_expected;

		if ( is_valid() )
		{
			new( &m_storage ) T( that.as_value() );
		}
		else
		{
			new( &m_storage ) std::exception_ptr( that.as_exception );
        }

        return *this;
    }

	inline expected&
	operator=(expected&& rhs)
	{
		swap( rhs );
		return *this;
	}

	expected( T const& value )
	:
		m_expected( true )
	{
		new( &m_storage ) T( value );
    }
	
    expected( T&& value )
	:
		m_expected( true )
	{
		new( &m_storage ) T( std::move( value ) );
    }

    template <typename E>
	inline expected(E const& exception, typename std::enable_if< std::is_base_of<std::exception, E>::value >::type* = nullptr )
	:
		m_expected( false )
	{
		if ( typeid( E ) != typeid( exception ) )
		{
			assert(false && "sliced!");
        }

		new( &m_storage ) std::exception_ptr( std::make_exception_ptr( exception ) );
    }

    expected( std::exception_ptr exception)
	:
		m_expected( false )
	{
		new( &m_storage ) std::exception_ptr( std::move( exception ) );
    }
/*
	template <typename F, typename = decltype(std::declval<F>()())>
	inline expected( F function )
	:
		m_expected (true)
	{
		try
		{
			new( &m_storage ) T( function() );
		}
		catch (...)
		{
			m_expected  = false;
            new( &m_storage ) std::exception_ptr( std::current_exception() );
        }
    }
*/

	inline ~expected()
	{
        if ( is_valid() )
		{
			as_value().~T();
		}
		else
		{
            using std::exception_ptr;
            as_exception().~exception_ptr();
		}
	}
	
	inline bool
	is_valid() const
	{
		return m_expected;
    }

	inline T&
	get()
	{
		if ( !is_valid() )
		{
			std::rethrow_exception( as_exception() );
		}
		
		return as_value();
    }
	
	inline T const&
	get() const
	{
		if ( !is_valid() )
		{
			std::rethrow_exception( as_exception() );
		}
		
		return as_value();
	}

	template <typename E>
	inline std::exception_ptr
	get_exception()
	{
		static_assert(std::is_base_of<std::exception, E>::value, "E must be derived from std::exception.");

		if ( !is_valid() )
		{
			try
			{
				std::rethrow_exception( as_exception() );
			}
			catch ( E const& )
			{
				return as_exception();
			}
			catch (...)
			{
			}
		}

        return std::exception_ptr( nullptr );
	}

	inline void
	swap(expected& rhs)
	{
		if ( is_valid() )
		{
			if ( rhs.is_valid() )
			{
				using std::swap;
				
				swap( as_value(),  rhs.as_value() );
			}
			else
			{
				auto temp = std::move( rhs.as_exception() );

				new( &rhs.m_storage  ) T( std::move( as_value() ) );
				new( &m_storage ) std::exception_ptr( std::move( temp ) );

				std::swap( m_expected , rhs.m_expected );
            }
        }
		else
		{
			if ( rhs.is_valid() )
			{
				rhs.swap( *this );
			}
			else
			{
				std::swap( as_exception(), rhs.as_exception() );
			}
		}
	}
	
private:

	typedef typename std::aligned_storage< max_of<sizeof(T), sizeof(std::exception_ptr)>::value,
		max_of<std::alignment_of<T>::value, std::alignment_of<std::exception_ptr>::value>::value
		>::type storage;

	inline T&
	as_value()
	{
        return *reinterpret_cast< T* >( &m_storage );
    }
	
	inline T const&
	as_value() const
	{
		return *reinterpret_cast< const T* >( &m_storage );
    }

	inline std::exception_ptr&
	as_exception()
	{
        return *reinterpret_cast< std::exception_ptr* >( &m_storage );
    }
	
	inline std::exception_ptr const&
	as_exception() const
	{
        return *reinterpret_cast< const std::exception_ptr* >( &m_storage );
    }
	
    bool	m_expected;
	storage m_storage;
};


namespace detail {

    //! Workaround for a compiler bug in MSVC
    template <typename T, typename F>
    expected<T> make_expected_helper(F function)
	{
		try
		{
			return expected< T >( function() );
		}
		catch ( ... )
		{
			return expected< T >( std::current_exception() );
		}
	}
}

template <typename T>
void swap( expected< T >& lhs, expected< T >& rhs)
{
	lhs.swap(rhs);
}

}

#endif