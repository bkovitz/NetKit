#ifndef _netkit_runloop_mac_h
#define _netkit_runloop_mac_h

#include <NetKit/NKRunLoop.h>

namespace netkit {

class runloop_mac : public runloop
{
public:

	runloop_mac();
	
	virtual ~runloop_mac();

	virtual source
	create_source( int fd, event e, event_f f );
	
	virtual source
	create_source( std::time_t time, event_f f );
	
	virtual void
	schedule( source s );
	
	virtual void
	cancel( source s );

	virtual void
	dispatch_on_main_thread( dispatch_f f );

	virtual void
	run();
	
	virtual void
	stop();
};

}

#endif
