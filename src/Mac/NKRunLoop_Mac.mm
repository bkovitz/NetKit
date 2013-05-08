/*
 * Copyright (c) 2013, Porchdog Software Inc.
 * All rights reserved.
 *
 * Redistribution and use in event and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of event code must retain the above copyright notice, this
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
#include <NetKit/NKSocket.h>
#include <dispatch/dispatch.h>

using namespace netkit;

runloop::ref
runloop::main()
{
	static runloop::ref singleton = new runloop_mac;
	
	return singleton;
}


runloop_mac::runloop_mac()
{
	// Make sure there is at least one thing added to runloop
	
	int fd = ::socket( AF_INET, SOCK_STREAM, 0 );
	
	auto event = create( fd, runloop::event_mask::read );
	
	schedule( event, [=]( runloop::event e )
	{
	} );
}


runloop_mac::~runloop_mac()
{
}


runloop::event
runloop_mac::create( int fd, event_mask m )
{
	dispatch_source_t event = nullptr;
	
	if ( m == event_mask::write )
	{
		event = dispatch_source_create( DISPATCH_SOURCE_TYPE_WRITE, fd, 0, dispatch_get_main_queue() );
	}
	else if ( m == event_mask::read )
	{
		event = dispatch_source_create( DISPATCH_SOURCE_TYPE_READ, fd, 0, dispatch_get_main_queue() );
	}

	dispatch_source_set_cancel_handler( event, ^()
	{
	} );
	
	return event;
}


runloop::event
runloop_mac::create( std::time_t msec )
{
	auto event = dispatch_source_create( DISPATCH_SOURCE_TYPE_TIMER, 0, 0, dispatch_get_main_queue() );

	dispatch_source_set_timer( event, dispatch_time( DISPATCH_TIME_NOW, msec * NSEC_PER_SEC ), msec * NSEC_PER_MSEC, 0 );

	return event;
}


void
runloop_mac::modify( event e, std::time_t msec )
{
	auto event = reinterpret_cast< dispatch_source_t >( e );
	dispatch_source_set_timer( event, dispatch_time( DISPATCH_TIME_NOW, msec * NSEC_PER_SEC ), msec * NSEC_PER_MSEC, 0 );
}


void
runloop_mac::schedule( event e, event_f f )
{
	auto event = reinterpret_cast< dispatch_source_t >( e );
	
	dispatch_source_set_event_handler( event, ^()
	{
		f( event );
	} );
	
	dispatch_resume( event );
}


void
runloop_mac::suspend( event e )
{
	auto event = reinterpret_cast< dispatch_source_t >( e );
	dispatch_suspend( event );
}


void
runloop_mac::cancel( event e )
{
	auto event = reinterpret_cast< dispatch_source_t >( e );
	dispatch_source_cancel( event );
	
	dispatch( [=]()
	{
	//	dispatch_release( event );
	} );
}


void
runloop_mac::dispatch( dispatch_f f )
{
	dispatch_async( dispatch_get_main_queue(), ^()
	{
		f();
	} );
}


void
runloop_mac::run( mode how )
{
	CFRunLoopRun();
}

	
void
runloop_mac::stop()
{
	CFRunLoopStop( CFRunLoopGetCurrent() );
}