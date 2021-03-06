#include <NetKit/NKLog.h>
#include <NetKit/NKUnicode.h>
#include "EventLog.h"
#include <winsock2.h>
#include <windows.h>
#include <stdarg.h>
#include <stdio.h>
#include <vector>
#include <time.h>
#include <mutex>

using namespace netkit;

static HANDLE g_event_source	= nullptr;

void
log::init( LPCTSTR name )
{
	HKEY key = NULL;

	if ( !m_set_handlers )
	{
		m_set_handlers = new set_handlers;
	}

	if ( !m_mutex )
	{
		m_mutex = new std::recursive_mutex;
	}

	if ( g_event_source == nullptr )
	{
		std::wstring	fullname;
		TCHAR			path[ MAX_PATH ];
		int				typesSupported;
		int				err;
		int				n;
	

		// Build the path string using the fixed registry path and app name.
	
		fullname = std::wstring( TEXT( "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\" ) ) + name;
	
    	// Add/Open the source name as a sub-key under the Application key in the EventLog registry key.
	
    	err = RegCreateKeyEx( HKEY_LOCAL_MACHINE, fullname.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &key, NULL );
   	 
    	if ( err )
    	{
			goto exit;
		}
	
    	// Set the path in the EventMessageFile subkey. Add 1 to the TCHAR count to include the null terminator.
	
    	n = GetModuleFileName( NULL, path, sizeof( path ) / sizeof( TCHAR ) );
   	 
    	if ( n == 0 )
    	{
			goto exit;
		}
	
    	n += 1;
    	n *= sizeof( TCHAR );
	
    	err = RegSetValueEx( key, TEXT( "EventMessageFile" ), 0, REG_EXPAND_SZ, (const LPBYTE) path, n );
   	 
    	if ( err )
    	{
			goto exit;
		}
	
    	// Set the supported event types in the TypesSupported subkey.

    	typesSupported = EVENTLOG_SUCCESS | EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE |  EVENTLOG_AUDIT_SUCCESS | EVENTLOG_AUDIT_FAILURE;
    	err = RegSetValueEx( key, TEXT( "TypesSupported" ), 0, REG_DWORD, (const LPBYTE) &typesSupported, sizeof( DWORD ) );
	
		if ( err )
		{
			goto exit;
		}
	
		g_event_source = RegisterEventSource( NULL, name );
	}

exit:

	if ( key )
	{
		RegCloseKey( key );
	}
}


void
log::put( log::level l, const char * filename, const char * function, int line, const char * format, ... )
{
	std::lock_guard< std::recursive_mutex > guard( *m_mutex );

	static char buf[ 32000 ];
	static char msg[ 32512 ];
	static char timeStr[ 1024 ];
	time_t t;
	int n = 0; 
	va_list ap;

	va_start( ap, format );
	n = vsnprintf_s( buf, sizeof( buf ), _TRUNCATE, format, ap );
	va_end( ap );
		
	t = time( NULL );
	ctime_s( timeStr, sizeof( timeStr ), &t );
		
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
		
	_snprintf_s( msg, sizeof( msg ), _TRUNCATE, "%d:%d %s %s:%d %s %s", GetCurrentProcessId(), GetCurrentThreadId(), timeStr, prune( filename ).c_str(), line, function, buf );
		
	for ( unsigned i = 0; i < strlen( msg ); i++ )
	{
		if ( msg[ i ] == '\n' )
		{
			msg[ i ] = '\0';
		}
	}

	if ( g_event_source )
	{
		WORD		type;
		const char	*array[ 1 ];
		BOOL		ok;

		// Map the debug level to a Windows EventLog type.
	
		if ( l == log::warning )
		{
			type = EVENTLOG_WARNING_TYPE;
		}
		else if ( l == log::error )
		{
			type = EVENTLOG_ERROR_TYPE;
		}
		else
		{
			type = EVENTLOG_INFORMATION_TYPE;
		}
	
		// Add the the string to the event log.
	
		array[ 0 ] = msg;
	
		ok = ReportEventA( g_event_source, type, 0, NETKIT_LOG, NULL, 1, 0, array, NULL );
	}

	OutputDebugStringA( msg );
	OutputDebugStringA( "\n" );
		
	fprintf( stderr, "%s\n", msg );
}
