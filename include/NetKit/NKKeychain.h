#ifndef _netkit_keychain_h
#define _netkit_keychain_h

#include <NetKit/NKObject.h>
#include <NetKit/NKError.h>
#include <string>

namespace netkit {

class keychain : public netkit::object
{
public:

	typedef smart_ref< keychain > ref;

	static keychain::ref
	instance();

	virtual netkit::status
	store( const std::string &url, const std::string &secret, std::string &key ) = 0;

	virtual netkit::status
	lookup( const std::string &key, std::string &secret ) = 0;
};

}


#endif
