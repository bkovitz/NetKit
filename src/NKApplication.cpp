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

#include <NetKit/NKApplication.h>
#include <NetKit/NKJSON.h>
#include <NetKit/NKLog.h>
#include <limits>
#include <sstream>
#include <vector>


using namespace netkit;


application::application( const std::string &name, const option::list &options, int argc, char **argv )
:
	m_okay( true )
{
	log::init( name );
	
#if defined( DEBUG )
	log::set_level( log::verbose );
#else
	log::set_level( log::info );
#endif

	for ( auto it = options.begin(); it != options.end(); it++ )
	{
		m_options[ ( *it )->name() ] = *it;
	}
	
	parse_command_line( argc, argv );
}


application::~application()
{
}


bool
application::parse_command_line( int argc, std::tchar_t **argv )
{
	for ( auto i = 1; i < argc; i++ )
	{
		auto it = m_options.find( argv[ i ] );
		
		if ( it != m_options.end() )
		{
			while ( ( ++i < argc ) && ( argv[ i ][ 0 ] != '-' ) )
			{
				it->second->set_is_set( true );
				
				if ( !it->second->push_back( argv[ i ] ) )
				{
					nklog( log::error, "syntax error for option '%'", it->second->name().c_str() );
					m_okay = false;
					break;
				}
			}
			
			if ( !it->second->set_is_set( true ) )
			{
				nklog( log::error, "syntax error for option '%'", it->second->name().c_str() );
				m_okay = false;
				break;
			}
		}
		else
		{
			nklog( log::error, "unknown option '%'", argv[ i ] );
			m_okay = false;
			break;
		}
	}
	
	return m_okay;
}