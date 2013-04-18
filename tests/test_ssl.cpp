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
 
#include "catch.hpp"
#include <NetKit/NetKit.h>

using namespace netkit;

static const char* g_message = "GET /index.html HTTP/1.1\r\nHost: collobos.com\r\n\r\n";

TEST_CASE( "NetKit/ssl", "ssl tests" )
{
	SECTION( "constructors", "socket constructors" )
	{
		netkit::ip::tcp::socket::ptr	sock = new netkit::ip::tcp::socket;
		REQUIRE( sock );
		
		sock->add( tls::adapter::create() );
		
		sock->connect( new uri( "https://www.collobos.com/index.html"), [=]( int status ) mutable
		{
			REQUIRE( status == 0 );
			
			auto num = sock->send( ( const std::uint8_t* ) g_message, strlen( g_message ) );
			
			fprintf( stderr, "wrote %lu bytes\n", num );
		} );
		
		sock->recv( [=]( int status, const std::uint8_t *buf, std::size_t len )
		{
			fprintf( stderr, "read %lu bytes: %s\n", len, buf );
			
			netkit::runloop::instance()->stop();
			
			return true;
		} );
		
		netkit::runloop::instance()->run();
	}
}
