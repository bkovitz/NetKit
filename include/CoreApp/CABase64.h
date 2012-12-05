#ifndef _coreapp_base64_h
#define _coreapp_base64_h

#include <coreapp/types.h>
#include <coreapp/tstring.h>

namespace coreapp {
namespace codec {

class base64
{
public:

	static std::tstring
	encode( const std::tstring &s );

	static std::tstring
	decode( const std::tstring &s );
};

}
}

#endif
