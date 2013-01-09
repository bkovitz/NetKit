#ifndef _coreapp_base64_h
#define _coreapp_base64_h

#include <CoreApp/CATypes.h>
#include <string>

namespace coreapp {
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
