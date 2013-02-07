#include <CoreApp/CALog.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <mutex>

using namespace coreapp;

static std::mutex	g_mutex;
static log::level	g_log_level = log::info;

void
log::init( const char *name )
{
}


log::level
log::get_level()
{
	std::lock_guard<std::mutex> lk( g_mutex );

	return g_log_level;
}


void
log::set_level( log::level l )
{
	std::lock_guard<std::mutex> lk( g_mutex );

	g_log_level = l;
}


void
log::put( log::level l, const char * filename, const char * function, int line, const char * format, ... )
{
	std::lock_guard<std::mutex> lk( g_mutex );

	if ( l <= g_log_level )
	{
		char buf[ 1024 ];
		char msg[ 2048 ];
		char * timeStr;
		time_t t;
		va_list ap;

		va_start( ap, format );
		vsnprintf( buf, sizeof( buf ), format, ap );
		va_end( ap );
		
		t = time( NULL );
		timeStr = ctime( &t );
		
		if ( timeStr )
		{
			for ( unsigned i = 0; i < strlen( timeStr ); i++ )
			{
				if ( timeStr[ i ] == '\n' )
				{
					timeStr[ i ] = '\0';
				}
			}
		}
		
		snprintf( msg, sizeof( msg ), "%s %s:%d %s", timeStr, function, line, buf );
		fprintf( stderr, "%s\n", msg );
	}
}