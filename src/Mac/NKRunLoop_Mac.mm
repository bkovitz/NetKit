#include "NKRunLoop_Mac.h"

using namespace netkit;

runloop::ptr
runloop::instance()
{
	static runloop::ptr singleton = new runloop_impl;
	
	return singleton;
}


void
runloop_impl::register_for_event( object::ptr obj, event e, event_handler_f h )
{
}

	
void
runloop_impl::dispatch_on_main_thread( handler_f b )
{
}


void
runloop_impl::run()
{
}

	
void
runloop_impl::stop()
{
}