#ifndef _netkit_sink_h
#define _netkit_sink_h

#include <NetKit/NKObject.h>
#include <list>

namespace netkit {

class source;
typedef smart_ptr< source > source_ptr;

class sink : public object
{
public:

	typedef smart_ptr< sink > ptr;
	typedef std::list< sink > list;
	
	virtual int
	read( source_ptr s ) = 0;
};

}

#endif
