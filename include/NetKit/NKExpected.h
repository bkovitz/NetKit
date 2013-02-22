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

	inline ~expected()
	{
	/*
        if ( is_valid() )
		{
			as_value().~T();
		}
		else
		{
            using std::exception_ptr;
            as_exception().~exception_ptr();
		}
		*/
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
		return *reinterpret_cast< T* >( &m_storage );
    }

	inline std::exception_ptr&
	as_exception()
	{
        return *reinterpret_cast< std::exception_ptr* >( &m_storage );
    }
	
	inline std::exception_ptr const&
	as_exception() const
	{
        return *reinterpret_cast< std::exception_ptr* >( &m_storage );
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

template <typename T, typename... Args>
inline expected<T>
make_expected(Args&&... args)
{
    return detail::make_expected_helper<T>([&]
	{
		return T( std::forward< Args >( args )... );
    });
}


template <typename T>
void swap( expected< T >& lhs, expected< T >& rhs)
{
	lhs.swap(rhs);
}

}

#endif