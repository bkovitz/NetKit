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
 
#ifndef _netkit_application_h
#define _netkit_application_h

#include <NetKit/NKJSON.h>
#include <NetKit/NKString.h>
#include <unordered_map>
#include <memory>

namespace netkit {

class application
{
public:

	class option
	{
	public:
	
		typedef std::shared_ptr< option >	ref;
		typedef std::vector< ref >			list;
	
		option( const std::string &name, std::int32_t num_values )
		:
			m_name( name ),
			m_num_values( num_values )
		{
		}
	
	inline const std::string&
		name() const
		{
			return m_name;
		}
	
		inline std::size_t
		num_values() const
		{
			return m_num_values;
		}
		
		inline bool
		is_set() const
		{
			return m_set;
		}
		
		inline bool
		set_is_set( bool val )
		{
			bool ok = false;
			
			if ( m_values.size() == m_num_values )
			{
				m_set = val;
				ok = true;
			}
			
			return ok;
		}
	
		inline const std::string&
		string_at_index( std::size_t index )
		{
			return m_values[ index ];
		}
		
		inline int
		int_at_index( std::size_t index )
		{
			return std::stoi( m_values[ index ] );
		}
		
		inline bool
		push_back( const std::string &value )
		{
			bool ok = false;
			
			if ( m_values.size() < m_num_values )
			{
				m_values.push_back( value );
				ok = true;
			}
			
			return ok;
		}
			
	private:

		std::string					m_name;
		bool						m_set			= false;
		std::int32_t				m_num_values	= 0;
		std::vector< std::string >	m_values;
	};
	
	application( const std::string &name, const option::list &options, int argc, char **argv );

	virtual ~application();
	
	template < class T >
	inline typename T::ref
	lookup_option( const std::string &name )
	{
		auto			it = m_options.find( name );
		typename T::ref opt;
	
		if ( it != m_options.end() )
		{
			opt = std::dynamic_pointer_cast< T >( *it );
		}
		else
		{
			opt = std::make_shared< T >();
		}
		
		return opt;
	}
	
	inline bool
	is_okay() const
	{
		return m_okay;
	}
	
	virtual void
	run() = 0;

protected:

	bool m_okay;
	
private:
	
	bool
	parse_command_line( int argc, std::tchar_t **argv );

	typedef std::unordered_map< std::string, option::ref > options;
	
	options m_options;
};

}

inline netkit::application::option::list
operator+( const netkit::application::option::list &lhs, const netkit::application::option::list &rhs )
{
	netkit::application::option::list l = lhs;

	l.insert( l.end(), rhs.begin(), rhs.end() );

    return l;
}

#endif
