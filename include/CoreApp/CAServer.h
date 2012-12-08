#ifndef _coreapp_server_h
#define _coreapp_server_h

#include <CoreApp/CASocket.h>
#include <deque>

namespace coreapp {

class server : public object
{
public:

	typedef smart_ptr< server > ptr;
	typedef std::deque< ptr > list;
	
	server();
	
	virtual ~server() = 0;
	
	virtual bool
	adopt( const socket::ptr &sock, uint8_t *peek, size_t len ) = 0;
};

}

#endif
