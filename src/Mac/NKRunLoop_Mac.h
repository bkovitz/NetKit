#ifndef _netkit_runloop_mac_h
#define _netkit_runloop_mac_h

#include <NetKit/NKRunLoop.h>

namespace netkit {

class runloop_impl : public runloop
{
public:

	virtual void
	register_for_event( object::ptr obj, event e, event_handler_f h );
	
	virtual void
	dispatch_on_main_thread( handler_f b );

	virtual void
	run();
	
	virtual void
	stop();
};

}

#endif
