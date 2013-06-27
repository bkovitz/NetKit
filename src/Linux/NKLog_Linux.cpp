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
#include <syslog.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <vector>
#include <mutex>

using namespace netkit;

static const int FACILITY = LOG_DAEMON;

void
log::init( const char *name )
{
	openlog( name, LOG_PID, FACILITY );
}


void
log::put( log::level l, const char * filename, const char * function, int line, const char * format, ... )
{
	std::lock_guard<std::recursive_mutex> lock( *m_mutex );

	if ( l <= m_log_level )
	{
		int 		level;
		static char msg[ 32767 ];
		static char buf[ 32767 ];
		va_list		ap;
		
		switch ( l )
		{
		case log::error:
			level = LOG_ERR;
			break;
		case log::warning:
			level = LOG_WARNING;
			break;
		case log::info:
			level = LOG_INFO;
			break;
		default:
			level = LOG_DEBUG;
		}

		int priority = FACILITY | level;

		va_start( ap, format );
		vsnprintf( buf, sizeof( buf ), format, ap );
		va_end( ap );

		snprintf( msg, sizeof( msg ), "%d %s:%d %s", getpid(), function, line, buf );

		syslog( priority, msg );
	}
}
