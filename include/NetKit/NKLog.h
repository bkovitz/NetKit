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
 
#ifndef _netkit_log_h
#define _netkit_log_h

#include <NetKit/NKObject.h>
#include <NetKit/NKCookie.h>
#include <NetKit/NKPrintF.h>
#include <sstream>
#include <vector>
#include <mutex>

#if defined( WIN32 )

#	include <winsock2.h>
#	include <windows.h>
#	include <stdio.h>
#	define nklog( LEVEL, MESSAGE, ... ) if ( LEVEL <= log::get_level() ) { try { netkit::log::put( LEVEL, __FILE__, __FUNCTION__, __LINE__, MESSAGE, __VA_ARGS__ ); } catch ( ... ) { fprintf( stderr, "logging exception at %s:%d\n", __FUNCTION__, __LINE__ ); } }

#elif defined( __clang__ )

#	define nklog( LEVEL, MESSAGE, ... ) if ( LEVEL <= log::get_level() ) { try { netkit::log::put( LEVEL, __FILE__, __PRETTY_FUNCTION__, __LINE__, MESSAGE, ##__VA_ARGS__ ); } catch ( ... ) { fprintf( stderr, "logging exception at %s:%d\n", __PRETTY_FUNCTION__, __LINE__ ); } };

#endif


namespace netkit {

class NETKIT_DLL log
{
public:

	enum level
	{
		error		= 1,
		warning		= 2,
		info		= 3,
		verbose		= 10,
		voluminous	= 20
	};
		
	typedef std::function< void ( level l ) > set_f;

	static void
#if defined( WIN32 )
	init( LPCTSTR name );
#else
	init( const std::string &name );
#endif

	inline static level
	get_level()
	{
		return m_log_level;
	}

	static void
	set_level( level l );

	static void
	on_set( cookie::ref *cookie, set_f handler );
	
	template< typename ...Params >
	static void
	put( level l, const char *filename, const char *function, int line, const char *format, const Params &... params )
	{
		std::ostringstream	os;
		char				*time_str;
		auto				t = time( NULL );
			
		time_str = ctime( &t );
			
		if ( time_str )
		{
			for ( unsigned i = 0; i < strlen( time_str ); i++ )
			{
				if ( time_str[ i ] == '\n' )
				{
					time_str[ i ] = '\0';
				}
			}
		}
			
		os << getpid() << " " << time_str << " " << prune_filename( filename ) << ":" << line << " " << prune_function( function ) << " ";
			
		netkit::printf( os, format, params... );
		
		put( l, os );
	}
	
	static void
	put( level l, std::ostringstream &os );
	
protected:

	static std::string
	prune_filename( const char *filename );

	static std::string
	prune_function( const char *filename );
	
	typedef std::vector< std::pair< netkit::cookie::naked_ptr, set_f > >	set_handlers;
	static log::level														m_log_level;
	static set_handlers														*m_set_handlers;
	static std::recursive_mutex												*m_mutex;
};

}

#endif
