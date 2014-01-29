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
#include <NetKit/NKIntrusiveList.h>
#include <NetKit/NKCookie.h>
#include <queue>
#include <list>
#include <ios>

namespace netkit {

class NETKIT_DLL source : public object
{
public:

	typedef std::function< void ( int status, const endpoint::ref &peer ) >					connect_reply_f;
	typedef std::function< void ( int status ) >											send_reply_f;
	typedef std::function< void ( int status, const std::uint8_t *buf, std::size_t len ) >	recv_reply_f;
	typedef std::function< void ( void ) >													close_f;
	typedef smart_ref< source >																ref;
	
	class adapter
	{
	public:
	
		DECLARE_INTRUSIVE_LIST_OBJECT( adapter )
		
		typedef std::function< void ( int status, const uri::ref &uri,  ip::address::list &addrs ) >						resolve_reply_f;
		typedef std::function< void ( int status ) >																		connect_reply_f;
		typedef std::function< void ( int status, const std::uint8_t *out_buf, std::size_t out_len ) >						send_reply_f;
		typedef std::function< void ( int status, const std::uint8_t *out_buf, std::size_t out_len, bool more_coming ) >	recv_reply_f;
	
		typedef adapter						*ref;
		typedef intrusive_list< adapter >	list;
		
		adapter();
		
		virtual ~adapter();
		
		virtual void
		resolve( const uri::ref &uri, resolve_reply_f reply );
	
		virtual void
		connect( const uri::ref &uri, const endpoint::ref &to, connect_reply_f reply );
		
		virtual void
		send( const std::uint8_t *in_buf, std::size_t in_len, send_reply_f reply );
		
		virtual void
		recv( const std::uint8_t *in_buf, std::size_t in_len, recv_reply_f reply );

	protected:
	
		friend class source;
		
		source::ref m_source;
	};
	
	source();
	
	virtual ~source();

	virtual void
	add( adapter::ref adapter );
	
	void
	connect( const uri::ref &uri, connect_reply_f reply );
		
	void
	send( const std::uint8_t *buf, std::size_t len, send_reply_f reply );
	
	void
	send( adapter *adapter, const std::uint8_t *buf, std::size_t len, send_reply_f reply );
	
	void
	recv( recv_reply_f reply );
	
	virtual bool
	is_open() const = 0;
	
	virtual void
	close( bool notify = true );
	
	void
	on_close( netkit::cookie::ref *cookie, close_f func );
	
	virtual endpoint::ref
	peer() const;

	inline bool
	closed() const
	{
		return m_closed;
	}
	
protected:

	typedef std::list< std::pair< netkit::cookie::naked_ptr, close_f > >	close_handlers;
	typedef std::vector< std::uint8_t >										buf_t;

	struct send_info
	{
		send_info( const std::uint8_t *buf, std::size_t len )
		:
			m_buf( buf, buf + len ),
			m_idx( 0 )
		{
		}
	
		buf_t			m_buf;
		std::size_t		m_idx;
	};
	
	virtual void
	start_connect( const endpoint::ref &to, connect_reply_f reply ) = 0;

	virtual void
	start_send( const std::uint8_t *buf, std::size_t len, send_reply_f reply ) = 0;
	
	virtual void
	start_recv( recv_reply_f reply ) = 0;
	
	void
	handle_resolve( ip::address::list addrs, const uri::ref &uri, connect_reply_f reply );

	void
	connect_internal( const uri::ref &uri, const endpoint::ref &to, connect_reply_f reply );
	
	void
	recv_internal( recv_reply_f reply );
	
	void
	teardown_notifications();
	
	typedef std::queue< send_info* >	send_queue;
	typedef std::queue< buf_t >			recv_queue;

	adapter::list	m_adapters;
	close_handlers	m_close_handlers;
	
	send_queue		m_send_queue;
	recv_queue		m_recv_queue;
	buf_t			m_recv_buf;

	bool			m_closed;
};

}

#endif
