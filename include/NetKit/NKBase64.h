#ifndef _netkit_base64_h
#define _netkit_base64_h

#include <string>

namespace netkit {

namespace codec {

class base64
{
public:

	static std::string
	encode( const std::string &s );

	static std::string
	decode( const std::string &s );
};

}

}

#endif
