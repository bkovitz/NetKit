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
 
#ifndef _netkit_source_h
#define _netkit_source_h

#include <NetKit/NKRunLoop.h>
#include <NetKit/NKEndpoint.h>
#include <list>
#include <ios>

namespace netkit {

class source : public object
{
public:

	typedef std::function< void ( int status ) >											connect_reply_f;
	typedef std::function< void ( int status ) >											accept_reply_f;
	typedef std::function< bool ( int status, const std::uint8_t *buf, std::size_t len ) >	peek_reply_f;
	typedef std::function< bool ( int status, const std::uint8_t *buf, std::size_t len ) >	recv_reply_f;
	typedef std::function< void ( void ) >													close_f;
	typedef smart_ptr< source >																ptr;
	
	class adapter
	{
	public:
	
		typedef adapter *ptr;
		
		adapter();
		
		virtual ~adapter();
		
		virtual void
		connect( const endpoint::ptr &endpoint, connect_reply_f reply ) = 0;
		
		virtual void
		accept( accept_reply_f reply ) = 0;
		
		virtual void
		peek( peek_reply_f reply ) = 0;
	
		virtual void
		recv( recv_reply_f reply ) = 0;
	
		virtual std::streamsize
		send( const std::uint8_t *buf, std::size_t len ) = 0;
		
	protected:
	
		friend class source;
		
		source::ptr		m_source;
		adapter::ptr	m_next;
	};
	
	source();
	
	virtual ~source();

	virtual void
	add( adapter::ptr adapter );
	
	virtual tag
	bind( close_f func );
	
	virtual void
	unbind( tag t );
	
	void
	connect( const endpoint::ptr &endpoint, connect_reply_f reply );
		
	void
	accept( accept_reply_f reply );
	
	void
	peek( peek_reply_f reply );
	
	void
	recv( recv_reply_f reply );
	
	std::streamsize
	send( const std::uint8_t *buf, std::size_t len );
	
	virtual bool
	is_open() const = 0;
	
	virtual void
	close() = 0;
	
	inline runloop::event
	write_event() const
	{
		return m_write_event;
	}
	
	inline runloop::event
	read_event() const
	{
		return m_read_event;
	}
	
protected:

	typedef std::list< std::pair< tag, close_f > > close_handlers;
	
	close_handlers	m_close_handlers;
	adapter::ptr	m_adapters;
	runloop::event	m_write_event;
	runloop::event	m_read_event;
};

}

#endif
