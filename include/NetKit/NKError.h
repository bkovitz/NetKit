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
 
#ifndef _netkit_error_h
#define _netkit_error_h

#include <NetKit/NKObject.h>
#include <string>
#include <iostream>

namespace netkit {

enum class status
{
	ok					= 0,
	expired				= -32000,
	no_memory			= -32001,
	not_authorized		= -32002,
	write_failed		= -32003,
	not_implemented		= -32004,
	unexpected			= -32005,
	connection_aborted	= -32006,
	parse_error			= -32007,
	permission_denied	= -32008,
	limit_error			= -32009,
	network_error		= -32010,
	uninitialized		= -32011,
	component_failure	= -32012,	
	invalid				= -32600,
	not_found			= -32601,
	bad_params			= -32602,
	internal_error		= -32603,
};


typedef std::function< void ( netkit::status status ) > reply_f;

class scout
{
public:

	scout( reply_f reply )
	:
		m_status( status::ok ),
		m_reply( reply )
	{
	}

	~scout()
	{
		m_reply( m_status );
	}

	inline void
	set_status( netkit::status status )
	{
		m_status = status;
	}

private:

	netkit::status	m_status;
	reply_f			m_reply;
};


std::string NETKIT_DLL
status_to_string( status v );

}


std::ostream& NETKIT_DLL
operator<<(std::ostream &output, const netkit::status status );


#endif
