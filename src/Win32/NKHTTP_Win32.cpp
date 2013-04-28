#include "NKHTTP_Win32.h"
#include <NetKit/NKRunLoop.h>
#include <NetKit/NKPlatform.h>
#include <NetKit/NKUnicode.h>
#include <NetKit/NKLog.h>

#pragma comment(lib, "winhttp.lib")

using namespace netkit::http;

request::ref
request::create( std::uint16_t major, std::uint16_t minor, int method, const netkit::uri::ref &uri )
{
	return new request_win32( major, minor, method, uri );
}


request_win32::request_win32( std::uint16_t major, std::uint16_t minor, int method, const std::string &s )
:
	request( major, minor, method, new netkit::uri( s ) ),
	m_connect_handle( NULL ),
	m_handle( NULL )
{
}


request_win32::request_win32( std::uint16_t major, std::uint16_t minor, int method, const netkit::uri::ref& u )
:
	request( major, minor, method, u ),
	m_connect_handle( NULL ),
	m_handle( NULL )
{
}


request_win32::request_win32( const request_win32 &that )
:
	request( that ),
	m_connect_handle( NULL ),
	m_handle( NULL )
{
}


request_win32::~request_win32()
{
	if ( m_connect_handle )
	{
		WinHttpCloseHandle( m_connect_handle );
	}

	if ( m_handle )
	{
		WinHttpCloseHandle( m_handle );
	}
}


request::ref
request_win32::copy() const
{
	return new request_win32( *this );
}


response::ref
response::create( std::uint16_t major, std::uint16_t minor, std::uint16_t status, bool keep_alive )
{
	return new response_win32( major, minor, status, keep_alive );
}


response_win32::response_win32( std::uint16_t major, std::uint16_t minor, std::uint16_t status, bool keep_alive )
:
	response( major, minor, status, keep_alive )
{
}


response_win32::response_win32( const response_win32 &that )
:
	response( that )
{
}


response::ref
response_win32::copy() const
{
	return new response_win32( *this );
}


response_win32::~response_win32()
{
}

client_win32::client_win32( const request::ref &request, auth_f handler, response_f reply )
:
	client( request, handler, reply ),
	m_session_handle( NULL ),
	m_resolve_timeout( 0 ),
	m_connect_timeout( 60000 ),
	m_send_timeout( 30000 ),
	m_receive_timeout( 30000 ),
	m_request( ( request_win32* ) super::m_request.get() ),
	m_done( false )
{
	super::m_response = new http::response_win32( request->major(), request->minor(), http::status::ok, request->keep_alive() );
	m_response = ( ( response_win32* ) super::m_response.get() );
}


client_win32::~client_win32()
{
	if ( m_session_handle )
	{
		WinHttpCloseHandle( m_session_handle );
	}
}


void CALLBACK
client_win32::callback( HINTERNET handle, DWORD_PTR context, DWORD code, void* info, DWORD length )
{
	client_win32 *self = reinterpret_cast< client_win32* >( context );

	if ( self )
	{
		self->m_mutex.lock();

		switch ( code )
		{
			case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE:
			{
				runloop::instance()->dispatch_on_main_thread( [=]()
				{
					self->on_send_request_complete();
				} );

				self->m_mutex.unlock();
			}
			break;
	
			case WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE:
			{
				runloop::instance()->dispatch_on_main_thread( [=]()
				{
					self->on_headers_are_available();
				} );

				self->m_mutex.unlock();
			}
			break;

			case WINHTTP_CALLBACK_STATUS_READ_COMPLETE:
			{
				runloop::instance()->dispatch_on_main_thread( [=]()
				{
					self->on_read( length );
				} );

				self->m_mutex.unlock();
			}
			break;

			case WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING:
			{
				self->m_mutex.unlock();

				runloop::instance()->dispatch_on_main_thread( [=]()
				{
					self->on_closing( handle );
				} );
			}
			break;
	
			case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR:
			{
				WINHTTP_ASYNC_RESULT	*result = (WINHTTP_ASYNC_RESULT*) info;
				DWORD					error = result->dwError;

				self->m_mutex.unlock();

				runloop::instance()->dispatch_on_main_thread( [=]()
				{
					self->on_error( handle, error );
				} );
			}
	        break;

			default:
			{
				self->m_mutex.unlock();
			}
		}
	}
}


void
client::send( const request::ref &request, response_f response_func )
{
    client_win32 *self = new client_win32( request, [=]( request::ref &request, uint32_t status )
    {
        return false;
    }, response_func );

    self->send_request();
}


void
client_win32::send_request()
{
	DWORD		options;
	DWORD		open_flags = 0;
	HINTERNET	handle;

	retain();

	if ( m_request->uri()->host().size() <= 0 )
	{
		reply( ERROR_PATH_NOT_FOUND );
		goto exit;
	}

	if ( m_session_handle == NULL )
    {
		std::wostringstream os;

		os << TEXT( "NetKit/2 " ) << widen( platform::machine_description() );

		m_session_handle = ::WinHttpOpen( os.str().c_str(), WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, WINHTTP_FLAG_ASYNC );

		if ( m_session_handle == NULL )
        {
			reply( ::GetLastError() );
			goto exit;
		}

		if ( ::WinHttpSetStatusCallback( m_session_handle, callback, WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS, NULL ) == WINHTTP_INVALID_STATUS_CALLBACK )
		{
			reply( ::GetLastError() );
			goto exit;
		}

		// From here on in, we will not delete this object until we receive a closing notification,
		// so we'll do a retain here.

		retain();
	}

	// ::WinHttpSetTimeouts( m_session_handle, m_resolve_timeout, m_connect_timeout, m_send_timeout, m_receive_timeout );

	handle = ::WinHttpConnect( m_session_handle, widen( m_request->uri()->host() ).c_str(), m_request->uri()->port(), 0 );

	if ( handle == NULL )
	{
		reply( ::GetLastError() );
		goto exit;
	}

	m_request->set_connect_handle( handle );

	if ( m_request->uri()->scheme() == "https" )
	{
		open_flags = WINHTTP_FLAG_SECURE;
	}

	handle = ::WinHttpOpenRequest( m_request->connect_handle(), widen( method::to_string( m_request->method() ) ).c_str(), widen( m_request->uri()->path() ).c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, open_flags );

	if ( handle == NULL )
	{
		reply( ::GetLastError() );
		goto exit;
	}

	m_request->set_handle( handle );

	if ( m_request->heder().size() > 0 )
	{
		std::wstring headers = m_request->headers_to_string();

		if ( !::WinHttpAddRequestHeaders( m_request->handle(), headers.c_str(), headers.size(), WINHTTP_ADDREQ_FLAG_ADD | WINHTTP_ADDREQ_FLAG_REPLACE) )
		{
			reply( GetLastError() );
			goto exit;
		}
	}

	if ( m_request->proxy() )
	{
		WINHTTP_PROXY_INFO	proxy_info;
		std::wostringstream	os;
		std::wstring		address;

		os << widen( m_request->proxy()->host() ) << TEXT( ":" ) << m_request->proxy()->port();
		address = os.str();

		memset( &proxy_info, 0, sizeof( proxy_info ) );

		proxy_info.dwAccessType		= WINHTTP_ACCESS_TYPE_NAMED_PROXY;
		proxy_info.lpszProxy		= const_cast< LPWSTR >( address.c_str() );
		proxy_info.lpszProxyBypass	= const_cast< LPWSTR >( widen( m_request->proxy()->bypass() ).c_str() );

		if ( !::WinHttpSetOption( m_request->handle(), WINHTTP_OPTION_PROXY, &proxy_info, sizeof( proxy_info ) ) )
		{
			reply( GetLastError() );
			goto exit;
		}

        if ( m_request->proxy()->user().length() > 0 )
		{
			std::wstring temp( widen( m_request->proxy()->user() ) );

			if ( !::WinHttpSetOption( m_request->handle(), WINHTTP_OPTION_PROXY_USERNAME, ( LPVOID ) temp.c_str(), temp.size() * sizeof( wchar_t ) ) )
			{
				reply( GetLastError() );
				goto exit;
			}
		}

		if ( m_request->proxy()->password().length() > 0 )
		{
			std::wstring temp( widen( m_request->proxy()->password() ) );

			if (!::WinHttpSetOption( m_request->handle(), WINHTTP_OPTION_PROXY_PASSWORD, ( LPVOID ) temp.c_str(), temp.size() * sizeof( wchar_t ) ) )
			{
				reply( GetLastError() );
				goto exit;
			}
		}
	}

	if ( !m_request->redirect() )
	{
		DWORD disable = WINHTTP_DISABLE_REDIRECTS;

		if ( !::WinHttpSetOption( m_request->handle(), WINHTTP_OPTION_DISABLE_FEATURE, &disable, sizeof( disable ) ) )
		{
			reply( GetLastError() );
			goto exit;
		}
	}

	options = SECURITY_FLAG_IGNORE_CERT_CN_INVALID | SECURITY_FLAG_IGNORE_CERT_DATE_INVALID | SECURITY_FLAG_IGNORE_UNKNOWN_CA | SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE;

	WinHttpSetOption( m_request->handle(), WINHTTP_OPTION_SECURITY_FLAGS, &options, sizeof( DWORD ) );

	if ( !::WinHttpSendRequest( m_request->handle(), WINHTTP_NO_ADDITIONAL_HEADERS, 0, ( LPVOID ) &m_request->body()[ 0 ], m_request->body().size(), m_request->body().size(), ( DWORD_PTR ) this ) )
	{
		reply( GetLastError() );
		goto exit;
	}

exit:

	release();
}


void
client_win32::on_send_request_complete()
{
	nklog( log::verbose, "sendrequest complete" );

	if ( !::WinHttpReceiveResponse( m_request->handle(), 0 ) )
	{
		reply( ::GetLastError() );
		goto exit;
	}

exit:

	return;
}


void
client_win32::on_headers_are_available()
{
	DWORD		statusCode = 0;
    DWORD		statusCodeSize = sizeof(DWORD);
	BOOL		bResult;
	DWORD		dwSize;
	wchar_t		*szHeader = NULL;

	nklog( log::verbose, "headers available" );

	if ( !::WinHttpQueryHeaders( m_request->handle(), WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &statusCodeSize, WINHTTP_NO_HEADER_INDEX))
	{
		reply( GetLastError() );
		goto exit;
	}

	m_response->set_status( ( std::uint16_t ) statusCode );

	if ( ( statusCode == 401 ) || ( statusCode == 407 ) )
	{
		if ( m_auth_func( super::m_request, statusCode ) )
		{
			send_request();
			goto exit;
		}
		else
		{
			reply( statusCode );
			goto exit;
		}
	}

	bResult = ::WinHttpQueryHeaders( m_request->handle(), WINHTTP_QUERY_RAW_HEADERS_CRLF, WINHTTP_HEADER_NAME_BY_INDEX, NULL, &dwSize, WINHTTP_NO_HEADER_INDEX );

	if ( !bResult && ( ::GetLastError() != ERROR_INSUFFICIENT_BUFFER ) )
	{
		reply( GetLastError() );
		goto exit;
	}

	if ( dwSize )
	{
	}

	szHeader = new wchar_t[ dwSize ];

	if ( szHeader == NULL )
	{
	}

	memset(szHeader, 0, dwSize* sizeof(wchar_t));

	if ( !::WinHttpQueryHeaders( m_request->handle(), WINHTTP_QUERY_RAW_HEADERS_CRLF, WINHTTP_HEADER_NAME_BY_INDEX, szHeader, &dwSize, WINHTTP_NO_HEADER_INDEX ) )
	{
		reply( GetLastError() );
		goto exit;
	}

	start_read();
                            
exit:

	if ( szHeader )
	{
		delete[] szHeader;
	}
}


void
client_win32::on_read( DWORD length )
{
	if ( length > 0 )
	{
		m_response->write( m_scratch, length );
		start_read();
	}
	else
	{
		reply( 0 );
	}
}


void
client_win32::on_error( HANDLE handle, DWORD error )
{
	nklog( log::info, "on_error(handle = 0x%x, error = %d)", handle, error );
	nklog( log::info, "session handle = 0x%x", m_session_handle );	
	reply( error );
	release();
}


void
client_win32::on_closing( HINTERNET handle )
{
	nklog( log::info, "on_closing" );
	if ( m_done )
	{
		release();
	}
}


void
client_win32::start_read()
{
	if ( !::WinHttpReadData( m_request->handle(), m_scratch, sizeof( m_scratch ), nullptr ) )
	{
		nklog( log::error, "WinHttpReadData() failed: %d", GetLastError() );
		reply( ::GetLastError() );
		goto exit;
	}

exit:

	return;
}


void
client_win32::reply( DWORD error )
{
	std::lock_guard< std::recursive_mutex > guard( m_mutex );

	m_response_func( error, super::m_response );

	m_request->set_handle( NULL );

	m_done = true;
}