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

#include "NKRunLoop_Linux.h"
#include <NetKit/NKSocket.h>
#include <sys/epoll.h>
#include <unistd.h>

using namespace netkit;

runloop::ref
runloop::main()
{
	static runloop::ref singleton = new runloop_linux;
	
	return singleton;
}


runloop_linux::runloop_linux()
{
	m_epoll_instance_fd = ::epoll_create( 100 );
}


runloop_linux::~runloop_linux()
{
	::close( m_epoll_instance_fd );
}


runloop::event
runloop_linux::create( int fd, event_mask m )
{
	struct epoll_event ev;
	int err;

	switch ( m )
	{
		case event_mask::read:
			ev.events = EPOLLIN;
			break;
		
		case event_mask::write:
			ev.events = EPOLLOUT;
			break;
		
		case event_mask::oob:
			break;

		case event_mask::timer:
			break;
	}

	ev.data.fd = fd;

	err = ::epoll_ctl( m_epoll_instance_fd, EPOLL_CTL_ADD, fd, &ev);
	if ( err == -1 )
	{
		return nullptr;
	}
	else
	{
		return 1;
	}
}


runloop::event
runloop_linux::create( std::time_t msec )
{
	// TODO
	return nullptr;
}


void
runloop_linux::modify( event e, std::time_t msec )
{
	// TODO
}


void
runloop_linux::schedule( event e, event_f f )
{
}


void
runloop_linux::schedule_oneshot_timer( std::time_t msec, event_f func )
{
}


void
runloop_linux::suspend( event e )
{
}


void
runloop_linux::cancel( event e )
{
}


void
runloop_linux::dispatch( dispatch_f f )
{
}


void
runloop_linux::run( mode how )
{
}

	
void
runloop_linux::stop()
{
}
