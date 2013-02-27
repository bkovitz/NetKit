/*
 * Copyright (c) 2013, Porchdog Software Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of the FreeBSD Project.
 *
 */
 
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