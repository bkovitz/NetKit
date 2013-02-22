#ifndef _netkit_parser_h
#define _netkit_parser_h

#include <NetKit/NKSource.h>
#include <NetKit/NKSink.h>

namespace netkit {

class parser : public sink
{
public:

	typedef smart_ptr< parser > ptr;
};

}

#endif
