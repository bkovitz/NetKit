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

#include <NetKit/NetKit.h>
#include <sys/utsname.h>
#include <uuid/uuid.h>
#include <cstdint>
#include <sstream>
#include <string>

using namespace netkit;

void
netkit::initialize()
{
	static bool first = true;

	if ( first )
	{
		netkit::component::m_instances                = new netkit::component::list;

		netkit::http::server::m_connections           = new netkit::http::connection::list;

		netkit::json::server::m_connections           = new netkit::json::connection::list;
		netkit::json::server::m_notification_handlers = new netkit::json::server::notification_handlers;
		netkit::json::server::m_request_handlers      = new netkit::json::server::request_handlers;

		first = false;
	}
}


std::string
platform::machine_description()
{
	std::ostringstream oss;
	utsname machine;

	int ret = uname( &machine );
	if ( ret == 0 )
	{
		oss << machine.sysname << " " << machine.version << " (Kernel " <<
			machine.release << ")";
	}
	else
	{
		oss << "Unknown Machine";
	}

	return oss.str();
}


uuid::ref
uuid::create()
{
	uuid_t 			id;
	std::uint8_t 	data[ 16 ];

	uuid_generate( id );
	
	for ( int i = 0; i < 16; i++ )
	{
		data[ i ] = id[ i ];
	}

	return new uuid( data );
}
