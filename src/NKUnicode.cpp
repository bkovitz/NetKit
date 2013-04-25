#include <NetKit/NKUnicode.h>
#if defined( WIN32 )
#	include <WinSock2.h>
#	include <Windows.h>
#else
#endif

using namespace netkit;

std::string
netkit::narrow( const wchar_t *s )
{
	std::string ret;

#if defined( WIN32 )

	int n;

    n = WideCharToMultiByte( CP_UTF8, 0, s, -1, NULL, 0, NULL, NULL );

    if ( n > 0 )
    {
        char *utf8;

        try
        {
            utf8 = new char[ n ];
        }
        catch ( ... )
        {
            utf8 = NULL;
        }

        if ( utf8 )
        {
            n = WideCharToMultiByte( CP_UTF8, 0, s, -1, utf8, n, NULL, NULL );

            if ( n > 0 )
            {
                ret = utf8;
            }

            delete [] utf8;
        }
    }

#else
#endif

    return ret;
}


std::string
netkit::narrow( const std::wstring &s )
{
	return narrow( s.c_str() );
}


std::wstring
netkit::widen( const char *s )
{
	std::wstring ret;

#if defined( WIN32 )

    int n;

    n = MultiByteToWideChar( CP_UTF8, 0, s, -1, NULL, 0 );

    if ( n > 0 )
    {
        wchar_t *utf16;

        try
        {
            utf16 = new wchar_t[ n ];
        }
        catch ( ... )
        {
            utf16 = NULL;
        }

        if ( utf16 )
        {
            n = MultiByteToWideChar( CP_UTF8, 0, s, -1, utf16, n );

            if ( n > 0 )
            {
                try
                {
                    ret = utf16;
                }
                catch( ... )
                {
                }
            }

            delete [] utf16;
        }
    }

#else

#endif

    return ret;
}


std::wstring
netkit::widen( const std::string &s )
{
	return widen( s.c_str() );
}