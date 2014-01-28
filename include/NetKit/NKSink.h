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
 
#ifndef _netkit_sink_h
#define _netkit_sink_h

#include <NetKit/NKSource.h>
#include <NetKit/NKCookie.h>
#include <NetKit/NKProxy.h>
#include <NetKit/NKURI.h>
#include <string>
#include <ios>

namespace netkit {

class NETKIT_DLL sink : public object
{
public:

	typedef std::function< void ( void ) >	close_f;
	typedef smart_ref< sink >				ref;

	sink();

	virtual ~sink();
	
	virtual void
	bind( source::ref source );

	virtual void
	unbind();
	
	void
	connect( const uri::ref &uri, source::connect_reply_f reply );
	
	void
	send( const std::uint8_t *buf, std::size_t len, source::send_reply_f reply );

	bool
	is_open() const;
	
	virtual void
	close();
	
	cookie::ref
	on_close( close_f func );
	
	endpoint::ref
	peer() const;
	
protected:

	typedef std::list< std::pair< std::uint64_t, close_f > > close_handlers;

	virtual bool
	process( const std::uint8_t *buf, std::size_t len ) = 0;

	void
	run();

	void
	source_was_closed();
	
	close_handlers		m_close_handlers;
	netkit::cookie::ref	m_on_close;
	source::ref			m_source;
	std::uint8_t		m_buf[ 4192 ];
};

}

#endif