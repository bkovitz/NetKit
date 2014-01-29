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
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <vector>
#include <mutex>

using namespace netkit;


log::level				log::m_log_level = log::info;
log::set_handlers		*log::m_set_handlers;
std::recursive_mutex	*log::m_mutex;

void
log::on_set( cookie::ref *cookie, set_f handler )
{
	std::lock_guard<std::recursive_mutex> lock( *m_mutex );
	
	if ( cookie )
	{
		*cookie = std::make_shared< netkit::cookie >( [=]( netkit::cookie::naked_ptr p )
		{
			std::lock_guard<std::recursive_mutex> lock( *m_mutex );
		
			for ( auto it = m_set_handlers->begin(); it != m_set_handlers->end(); it++ )
			{
				if ( it->first == p )
				{
					m_set_handlers->erase( it );
					break;
				}
			}
		} );

	   m_set_handlers->push_back( std::make_pair( cookie->get(), handler ) );
	}
	else
	{
	   m_set_handlers->push_back( std::make_pair( nullptr, handler ) );
	}
}


void
log::set_level( log::level l )
{
	std::lock_guard<std::recursive_mutex> lock( *m_mutex );
	
	if ( m_log_level != l )
	{
		m_log_level = l;
		
		for ( auto it = m_set_handlers->begin(); it != m_set_handlers->end(); it++ )
		{
			( it->second )( l );
		}
	}
}


std::string
log::prune( const char *filename )
{
	for ( auto i = strlen( filename ) - 1; i > 0; i-- )
	{
		if ( ( filename[ i ] == '/' ) || ( filename[ i ] == '\\' ) )
		{
			return filename + i + 1;
		}
	}

	return filename;
}
