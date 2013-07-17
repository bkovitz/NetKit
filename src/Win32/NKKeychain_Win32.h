#ifndef _netkit_keychain_win32_h
#define _netkit_keychain_win32_h

#include <NetKit/NKKeychain.h>

namespace netkit {

class keychain_win32 : public keychain
{
public:

	virtual netkit::status
	store( const std::string &url, const std::string &secret, std::string &key );

	virtual netkit::status
	lookup( const std::string &key, std::string &secret );
};

}

#endif
