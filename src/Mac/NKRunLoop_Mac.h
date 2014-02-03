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
 
#ifndef _netkit_runloop_mac_h
#define _netkit_runloop_mac_h

#include <NetKit/NKRunLoop.h>
#include <dispatch/dispatch.h>
#include <vector>
#include <stack>

namespace netkit {

class runloop_mac : public runloop
{
public:

	class fd_mac : public netkit::runloop::fd
    {
    public:
	
		struct accept_context
		{
			typedef std::shared_ptr< accept_context > ref;
			
			accept_context( std::size_t peek_len )
			:
				m_peek_buf( peek_len )
			{
			}
			
			~accept_context()
			{
			}
			
			std::vector< std::uint8_t > m_peek_buf;
			std::size_t					m_peek_len = 0;
		};
	
		struct send_context
		{
			typedef std::shared_ptr< send_context > ref;
			
			send_context( const std::uint8_t *buf, std::size_t len, send_reply_f reply )
			:
				m_buffer( buf, buf + len ),
				m_reply( reply )
			{
			}
			
			send_context( const std::uint8_t *buf, std::size_t len, const netkit::endpoint::ref &to, send_reply_f reply )
			:
				m_buffer( buf, buf + len ),
				m_reply( reply )
			{
				m_to_len = static_cast< socklen_t >( to->to_sockaddr( m_to ) );
			}
			
			~send_context()
			{
			}
			
			std::vector< std::uint8_t >	m_buffer;
			sockaddr_storage			m_to;
			socklen_t					m_to_len;
			std::uint32_t				m_bytes_written = 0;
			send_reply_f				m_reply;
		};
	
		typedef std::function< ssize_t ( send_context::ref &context ) > send_f;
	
		fd_mac( int fd, int domain );
		
		virtual ~fd_mac();
	
		virtual int
		bind( netkit::endpoint::ref to );

		virtual void
		connect( netkit::endpoint::ref to, connect_reply_f reply );

		virtual void
		accept( std::size_t peek, accept_reply_f reply );

		virtual void
		send( const std::uint8_t *buf, std::size_t len, send_reply_f reply );

		virtual void
		sendto( const std::uint8_t *buf, std::size_t len, netkit::endpoint::ref to, send_reply_f reply );

		virtual void
		recv( recv_reply_f reply );

		virtual void
		recvfrom( recvfrom_reply_f reply );

		virtual void
		close();
		
	private:
	
		inline void
		resume_send()
		{
			dispatch_resume( m_send_source );
			m_send_active = true;
		}
		
		inline void
		suspend_send()
		{
			dispatch_suspend( m_send_source );
			m_send_active = false;
		}
		
		inline void
		resume_recv()
		{
			dispatch_resume( m_recv_source );
			m_recv_active = true;
		}
		
		inline void
		suspend_recv()
		{
			dispatch_suspend( m_recv_source );
			m_recv_active = false;
		}
		
		void
		try_send( send_f func );
		
		inline void
		deliver_reply( const send_context::ref &context, int status )
		{
			context->m_reply( status );
			m_send_queue.pop();
		}
	
		std::stack< send_context::ref >	m_send_queue;
		std::vector< std::uint8_t >	m_out_buf;
		std::vector< std::uint8_t >	m_in_buf;
		int							m_domain;
		dispatch_source_t			m_send_source	= nullptr;
		bool						m_send_active	= false;
		dispatch_source_t			m_recv_source	= nullptr;
		bool						m_recv_active	= false;
		int							m_fd;
	};

	runloop_mac();
	
	virtual ~runloop_mac();
	
	virtual fd::ref
	create( std::int32_t domain, std::int32_t type, std::int32_t protocol );

	virtual fd::ref
	create( netkit::endpoint::ref in_endpoint, netkit::endpoint::ref &out_endpoint, std::int32_t domain, std::int32_t type, std::int32_t protocol );

	virtual event
	create( std::time_t msec );
	
	virtual void
	schedule( event e, event_f func );

	virtual void
	schedule_oneshot_timer( std::time_t msec, event_f func );
	
	virtual void
	suspend( event e );
	
	virtual void
	cancel( event e );
	
	virtual void
	dispatch( dispatch_f f );

	virtual void
	run( mode how = mode::normal );
	
	virtual void
	stop();
};

}

#endif
