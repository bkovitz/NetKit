#ifndef _netkit_unicode_h
#define _netkit_unicode_h

#include <string>

namespace netkit
{
	std::string
	narrow( const std::wstring &s );

	std::wstring
	widen( const std::string &s );
}

#endif
