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

#include <NetKit/NKError.h>

using namespace netkit;

std::string
netkit::status_to_string( netkit::status status )
{
	std::string s;
	
	switch ( status )
	{
		case netkit::status::ok:
		{
			static const char *msg = "None";
			s = msg;
		}
		break;
		
		case netkit::status::expired:
		{
			static const char *msg = "Expired";
			s = msg;
		}
		break;
		
		case netkit::status::no_memory:
		{
			static const char *msg = "Out Of Memory";
			s = msg;
		}
		break;
		
		case netkit::status::not_authorized:
		{
			static const char *msg = "Not Authorized";
			s = msg;
		}
		break;
		
		case netkit::status::write_failed:
		{
			static const char *msg = "Write Failed";
			s = msg;
		}
		break;
		
		case netkit::status::not_implemented:
		{
			static const char *msg = "Not Implemented";
			s = msg;
		}
		break;
		
		case netkit::status::unexpected:
		{
			static const char *msg = "Unexpected Error";
			s = msg;
		}
		break;
		
		case netkit::status::parse_error:
		{
			static const char *msg = "Parse Error";
			s = msg;
		}
		break;
		
		case netkit::status::invalid:
		{
			static const char *msg = "Invalid";
			s = msg;
		}
		break;
		
		case netkit::status::not_found:
		{
			static const char *msg = "Not Found";
			s = msg;
		}
		break;
		
		case netkit::status::bad_params:
		{
			static const char *msg = "Bad Parameters";
			s = msg;
		}
		break;
		
		case netkit::status::internal_error:
		{
			static const char *msg = "Internal Error";
			s = msg;
		}
		break;

		case netkit::status::permission_denied:
		{
			static const char *msg = "Permission Denied";
			s = msg;
		}
		break;

		case netkit::status::limit_error:
		{
			static const char *msg = "Exceeded Limit";
			s = msg;
		}
		break;
			
		case netkit::status::network_error:
		{
			static const char *msg = "Network Error";
			s = msg;
		}
		break;

		default:
		{
			static const char *msg = "Unknown Error";
			s = msg;
		}
	}
	
	return s;
}


std::ostream&
operator<<( std::ostream &output, const netkit::status status )
{
	return output << ( int ) status;
}
