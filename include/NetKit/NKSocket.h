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
 
#ifndef _netkit_socket_h
#define _netkit_socket_h

#include <NetKit/NKRunLoop.h>
#include <NetKit/NKSource.h>
#include <NetKit/NKSink.h>
#include <initializer_list>
#include <functional>
#include <errno.h>
#if defined( WIN32 )

#else
#	include <sys/socket.h>
#	include <unistd.h>
#	include <fcntl.h>
#endif

namespace netkit {

namespace socket {

#if defined( WIN32 )

typedef SOCKET native;
static const native null = INVALID_SOCKET;

#else

typedef int native;
static const native null = -1;

#endif

enum class error
{
#if defined( WIN32 )
	in_progress	=	WSAEINPROGRESS,
	would_block	=	WSAEWOULDBLOCK
#else
	in_progress	=	EINPROGRESS,
	would_block	=	EAGAIN
#endif
};


class server : public object
{
public:

	typedef std::function< sink::ptr ( source::ptr source, const std::uint8_t *buf, size_t len ) >	adopt_f;
	typedef std::list< adopt_f >																	adopters;

	int
	set_blocking( bool block );
	
	void
	bind( std::initializer_list< adopt_f > l );
	
protected:

	server( int domain, int type );

	server( native fd );
	
	server( const server &that );	// Not implemented
	
	adopters	m_adopters;
	native		m_fd;
};

	
class client : public source
{
public:

	typedef smart_ptr< client > ptr;

	inline runloop::source
	source() const
	{
		return m_source;
	}
	
	inline void
	set_source( runloop::source s )
	{
		m_source = s;
	}
	
	int
	set_blocking( bool block );
	
	inline bool
	is_open() const
	{
		return ( m_fd != null ) ? true : false;
	}
	
	virtual ssize_t
	peek( std::uint8_t *buf, size_t len ) = 0;
	
	virtual ssize_t
	recv( std::uint8_t *buf, size_t len ) = 0;
	
	virtual ssize_t
	send( const std::uint8_t *buf, size_t len ) = 0;
	
	virtual void
	close();
	
	inline native
	fd() const
	{
		return m_fd;
	}
	
protected:

	client( int domain, int type );

	client( native fd );
	
	client( const client &that );	// Not implemented
	
	virtual ~client();

	runloop::source	m_source;
	native			m_fd;
};

}

}

#endif