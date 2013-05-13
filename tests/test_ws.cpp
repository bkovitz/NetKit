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

TEST_CASE( "NetKit/ws", "websocket tests" )
{
	SECTION( "constructors", "websocket constructors" )
	{
		std::uint8_t	*buf = new std::uint8_t[ 1024 ];
		source::ref		sock = new netkit::ip::tcp::socket;
	
		REQUIRE( sock );
		
		sock->add( ws::client::create() );
		
		sock->connect( new uri( "ws://127.0.0.1:8080"), [=]( int status, const endpoint::ref &peer ) mutable
		{
			REQUIRE( status == 0 );
			
			sock->send( ( const std::uint8_t* ) "echo", 4, [=]( int status )
			{
				REQUIRE( status == 0 );
			} );
			
			sock->recv( buf, 1024, [=]( int status, std::size_t len ) mutable
			{
				REQUIRE( status == 0 );
				REQUIRE( strstr( ( const char* ) buf, "echo" ) != NULL );
				
				sock->send( ( const std::uint8_t* ) "hello", 5, [=]( int status ) mutable
				{
				} );
				
				sock->recv( buf, 1024, [=]( int status, std::size_t len ) mutable
				{
					REQUIRE( status == 0 );
					REQUIRE( strstr( ( const char* ) buf, "hello" ) != NULL );
				
					runloop::main()->stop();
				} );
			} );
		} );
		
		netkit::runloop::main()->run();
	}
}
