#ifndef _CoreApp_server_h
#define _CoreApp_server_h

#include "socket.h"
#include <deque>

namespace CoreApp {

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
