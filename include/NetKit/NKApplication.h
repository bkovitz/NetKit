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

// Forward declaration value

class application
{
public:

	class option
	{
	public:
	
		typedef std::shared_ptr< option > ref;
		typedef std::vector< ref > list;
	
		option( const std::string &name, std::uint8_t min_num_values, std::uint8_t max_num_values, bool required )
		:
			m_name( name ),
			m_min_num_values( min_num_values ),
			m_max_num_values( max_num_values ),
			m_required( required )
		{
		}
		
		~option()
		{
		}
		
		inline const std::string&
		name() const
		{
			return m_name;
		}
		
		inline const std::uint8_t
		min_num_values() const
		{
			return m_min_num_values;
		}
		
		inline const std::uint8_t
		max_num_values() const
		{
			return m_max_num_values;
		}
		
		inline bool
		required() const
		{
			return m_required;
		}
		
		inline json::value::ref&
		values()
		{
			return m_values;
		}
		
		inline const json::value::ref&
		values() const
		{
			return m_values;
		}
		
		inline bool
		is_set() const
		{
			return m_set;
		}
		
		inline void
		set_is_set( bool val )
		{
			m_set = val;
		}
		
	private:
	
		std::string			m_name;
		std::uint8_t		m_min_num_values;
		std::uint8_t		m_max_num_values;
		bool				m_required;
		json::value::ref	m_values;
		bool				m_set;
	};

	application( const std::string &name, const option::list &options, int argc, char **argv );

	virtual ~application();
	
	bool
	is_option_set( const std::string &name );
	
	bool
	is_option_set( const std::string &name, json::value::ref &values );
	
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
