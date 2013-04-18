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
 
#include <NetKit/NKSource.h>

using namespace netkit;

source::source()
{
}


source::~source()
{
}


void
source::add( adapter::ptr adapter  )
{
	adapter->m_source	= this;
	adapter->m_next		= m_adapters;
	m_adapters			= adapter;
}


tag
source::bind( close_f c )
{
	static int	tags	= 0;
	tag			t		= reinterpret_cast< tag >( ++tags );
	
	m_close_handlers.push_back( std::make_pair( t, c ) );
	
	return t;
}


void
source::unbind( tag t )
{
	for ( auto it = m_close_handlers.begin(); it != m_close_handlers.end(); it++ )
	{
		if ( it->first == t )
		{
			m_close_handlers.erase( it );
			break;
		}
	}
	
	for ( auto adapter = m_adapters; adapter; adapter = adapter->m_next )
	{
		if ( adapter == t )
		{
		}
	}
}


void
source::connect( const endpoint::ptr &endpoint, connect_reply_f reply )
{
	m_adapters->connect( endpoint, reply );
}

		
void
source::accept( accept_reply_f reply )
{
	m_adapters->accept( reply );
}


void
source::peek( peek_reply_f reply )
{
	return m_adapters->peek( reply );
}

	
void
source::recv( recv_reply_f reply )
{
	return m_adapters->recv( reply );
}

	
std::streamsize
source::send( const std::uint8_t *buf, size_t len )
{
	return m_adapters->send( buf, len );
}


source::adapter::adapter()
:
	m_next( nullptr )
{
}


source::adapter::~adapter()
{
}

