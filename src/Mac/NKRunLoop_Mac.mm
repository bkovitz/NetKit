#include "NKRunLoop_Mac.h"
#include <CoreFoundation/CoreFoundation.h>
#include <dispatch/dispatch.h>

using namespace netkit;

runloop::ptr
runloop::instance()
{
	static runloop::ptr singleton = new runloop_mac;
	
	return singleton;
}


runloop_mac::runloop_mac()
{
	// Make sure there is at least one thing added to runloop
	
	int fd = ::socket( AF_INET, SOCK_STREAM, 0 );
	
	auto source = create_source( fd, runloop::event::read, [=]( runloop::source s, runloop::event e )
	{
	} );
	
	schedule( source );
}


runloop_mac::~runloop_mac()
{
}


runloop::source
runloop_mac::create_source( int fd, event e, event_f f )
{
	auto source = dispatch_source_create( DISPATCH_SOURCE_TYPE_READ, fd, 0, dispatch_get_main_queue() );
	
	dispatch_source_set_event_handler( source,
	^()
	{
		f( source, runloop::event::read );
	} );
	
	dispatch_source_set_cancel_handler( source,
	^()
	{
	} );
	
	return source;
}


runloop::source
runloop_mac::create_source( std::time_t time, event_f f )
{
	auto source = dispatch_source_create( DISPATCH_SOURCE_TYPE_TIMER, 0, 0, dispatch_get_main_queue() );

	dispatch_source_set_timer( source, 0, ( ::time( NULL ) + time ) * NSEC_PER_SEC, 0 );

	dispatch_source_set_event_handler( source, ^()
	{
		f( source, runloop::event::timer );
	} );
	
	return source;
}

	
void
runloop_mac::schedule( source s )
{
	auto source = reinterpret_cast< dispatch_source_t >( s );
	dispatch_resume( source );
}


void
runloop_mac::cancel( source s )
{
	auto source = reinterpret_cast< dispatch_source_t >( s );
	dispatch_source_cancel( source );
	dispatch_release( source );
}


void
runloop_mac::dispatch_on_main_thread( dispatch_f f )
{
	dispatch_async( dispatch_get_main_queue(), ^()
	{
		f();
	} );
}


void
runloop_mac::run()
{
	CFRunLoopRun();
}

	
void
runloop_mac::stop()
{
	CFRunLoopStop( CFRunLoopGetMain() );
}