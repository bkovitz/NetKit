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
 
#ifndef _netkit_network_interface_h
#define _netkit_network_interface_h

#include <NetKit/NKAddress.h>
#include <vector>

namespace netkit {

class NETKIT_DLL nic : public object
{
public:

	struct flags
	{
		enum
		{
			up				= ( 1 << 0 ),
			loopback		= ( 1 << 1 ),
			point_to_point	= ( 1 << 2 ),
			multicast		= ( 1 << 3 ),
			broadcast		= ( 1 << 4 )
		};
	};
		
	typedef smart_ref< nic > ref;
	typedef std::vector< ref > array;

	static array
	instances();

	nic( const json::value_ref &root );

	inline const std::string&
	name() const
	{
		return m_name;
	}

	inline const std::string&
	display_name() const
	{
		return m_display_name;
	}

	inline const std::string&
	dns_suffix() const
	{
		return m_dns_suffix;
	}

	inline const netkit::ip::address::array&
	addresses() const
	{
		return m_addresses;
	}

	inline std::int32_t
	flags() const
	{
		return m_flags;
	}

	virtual bool
	equals( const object &that ) const;

	virtual void
	flatten( json::value_ref &root ) const;

	void
	inflate( const json::value_ref &root );

protected:

	nic();

	std::string 		m_name;
	std::string			m_display_name;
	std::string			m_dns_suffix;
	ip::address::array	m_addresses;
	std::int32_t		m_flags;
};

}

#endif