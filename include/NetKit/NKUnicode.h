#ifndef _netkit_unicode_h
#define _netkit_unicode_h

#include <NetKit/NKObject.h>
#include <string>

namespace netkit
{
	std::string NETKIT_DLL
	narrow( const wchar_t *s );

	std::string NETKIT_DLL
	narrow( const std::wstring &s );

	std::wstring NETKIT_DLL
	widen( const char *s );

	std::wstring NETKIT_DLL
	widen( const std::string &s );
}

#endif
