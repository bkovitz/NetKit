#ifndef _netkit_http_win32_h
#define _netkit_http_win32_h

#include <NetKit/NKHTTP.h>
#include <NetKit/NKUnicode.h>
#include <winhttp.h>
#include <mutex>

namespace netkit {

namespace http {

class request_win32 : public request
{
public:

	typedef smart_ref< request_win32 > ref;

	request_win32( std::uint16_t major, std::uint16_t minor, int method, const std::string &uri );
	
	request_win32( std::uint16_t major, std::uint16_t minor, int method, const uri::ref &uri );

	request_win32( const request_win32 &that );

	virtual ~request_win32();

	request::ref
	copy() const;

	inline HINTERNET
	connect_handle() const
	{
		return m_connect_handle;
	}

	inline void
	set_connect_handle( HINTERNET handle )
	{
		if ( m_connect_handle != NULL )
		{
			WinHttpCloseHandle( m_connect_handle );
		}

		m_connect_handle = handle;
	}

	inline HINTERNET
	handle() const
	{
		return m_handle;
	}

	inline void
	set_handle( HINTERNET handle )
	{
		if ( m_handle != NULL )
		{
			WinHttpCloseHandle( m_handle );
		}

		m_handle = handle;
	}

	inline std::wstring
	headers_to_string() const
	{
		std::wostringstream os;
		bool first = true;

		for ( message::header::const_iterator it = m_header.begin(); it != m_header.end(); it++ )
		{
			if ( !first )
			{
				os << "\r\n";
			}

			os << widen( it->first ) << ": " << widen( it->second );

			first = false;
		}

		return os.str();
	}

private:

	HINTERNET	m_connect_handle;
	HINTERNET	m_handle;
};


class response_win32 : public response
{
public:

	typedef smart_ref< response_win32 > ref;

	response_win32( std::uint16_t major, std::uint16_t minor, std::uint16_t status, bool keep_alive );

	response_win32( const response_win32 &that );

	virtual ~response_win32();

	response::ref
	copy() const;

	char buf[ 4192 ];

	std::size_t m_size;
};


class client_win32 : public client
{
public:

	typedef smart_ref< client_win32 > ref;
	typedef client super;

	client_win32( const request::ref &request, auth_f handler, response_f reply );

	void
	send_request();

	virtual ~client_win32();

private:

	static void CALLBACK
	callback( HINTERNET handle, DWORD_PTR context, DWORD code, void* info, DWORD length );

	void
	on_send_request_complete();

	void
	on_headers_are_available();

	void
	on_read( DWORD length );

	void
	on_error( HINTERNET handle, DWORD error );

	void
	on_closing( HINTERNET handle );

	void
	start_read();

	void
	reply( DWORD error );

	request_win32::ref		m_request;
	std::string				m_body;
	response_win32::ref		m_response;
	std::uint8_t			m_scratch[ 4192 ];
	HINTERNET				m_session_handle;
	uint32_t				m_resolve_timeout;
	uint32_t				m_connect_timeout;
	uint32_t				m_send_timeout;
	uint32_t				m_receive_timeout;
	std::recursive_mutex	m_mutex;
	bool					m_done;
};

}

}

#endif
