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

}

}

#endif
