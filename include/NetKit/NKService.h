#ifndef _netkit_server_h
#define _netkit_server_h

#include <CoreNetwork/CNSocket.h>
#include <deque>

namespace netkit {

class service : public object
{
public:

	typedef smart_ptr< service > ptr;
	typedef std::deque< ptr > list;
	
	service();
	
	virtual ~service() = 0;
	
	virtual bool
	adopt( const socket::ptr &sock, uint8_t *peek, size_t len ) = 0;
};

}

#endif
