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
 
#include <NetKit/NKLog.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <vector>
#include <mutex>

using namespace netkit;

void
log::init( const std::string &name )
{
	if ( !m_set_handlers )
	{
		m_set_handlers = new set_handlers;
	}
	
	if ( !m_mutex )
	{
		m_mutex = new std::recursive_mutex;
	}
}


void
log::put( log::level l, std::ostringstream &os )
{
	std::lock_guard< std::recursive_mutex > lock( *m_mutex );
	
	switch ( l )
	{
		case log::level::info:
		{
			fprintf( stderr, "      INFO %s\n", os.str().c_str() );
		}
		break;
		
		case log::level::warning:
		{
			fprintf( stderr, "   WARNING %s\n", os.str().c_str() );
		}
		break;
		
		case log::level::error:
		{
			fprintf( stderr, "     ERROR %s\n", os.str().c_str() );
		}
		break;
		
		case log::level::verbose:
		{
			fprintf( stderr, "   VERBOSE %s\n", os.str().c_str() );
		}
		break;
		
		case log::level::voluminous:
		{
			fprintf( stderr, "VOLUMINOUS %s\n", os.str().c_str() );
		}
		break;
	}
}