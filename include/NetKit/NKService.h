#ifndef _netkit_service_h
#define _netkit_service_h

#include <NetKit/NKSocket.h>
#include <deque>

namespace netkit {

class service : public object
{
public:

	typedef smart_ptr< service > ptr;
	typedef std::deque< ptr > list;
	
	service();
	
	virtual ~service();
};

}

#endif
