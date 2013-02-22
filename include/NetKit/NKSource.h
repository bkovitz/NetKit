#ifndef _netkit_source_h
#define _netkit_source_h

#include <NetKit/NKObject.h>
#include <initializer_list>
#include <list>

namespace netkit {

class sink;
typedef smart_ptr< sink > sink_ptr;

class source : public object
{
public:

	typedef smart_ptr< source > ptr;
	typedef std::list< sink > sink_list;
	
	void
	bind( std::initializer_list< sink_ptr > l );
	
protected:

	sink_list m_sinks;
};

}

#endif
