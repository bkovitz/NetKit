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


static const char* g_message = "GET /index.html HTTP/1.1\r\nHost: www.collobos.com\r\n\r\n";

TEST_CASE( "NetKit/ssl/1", "ssl client" )
{
	SECTION( "constructors", "socket constructors" )
	{
		std::uint8_t *buf = new std::uint8_t[ 4192 ];
		source::ref	sock = new netkit::ip::tcp::socket;
		REQUIRE( sock );
		
		sock->add( tls::client::create() );
		
		fprintf( stderr, "connecting ssl socket\n" );
		sock->connect( new uri( "https://www.collobos.com/index.html"), [=]( int status, const endpoint::ref &peer ) mutable
		{
			REQUIRE( status == 0 );
			
			sock->send( ( const std::uint8_t* ) g_message, strlen( g_message ), [=]( int status )
			{
				REQUIRE( status == 0 );
			} );
			
			sock->recv( [=]( int status, const std::uint8_t* buf, std::size_t len )
			{
				REQUIRE( status == 0 );
				REQUIRE( strstr( ( const char* ) buf, "HTTP/1.1 200 OK" ) != NULL );
				
				runloop::main()->stop();
			} );
		} );
		
		netkit::runloop::main()->run();
	}
}

TEST_CASE( "NetKit/ssl/2", "ssl server" )
{
	SECTION( "constructors", "socket constructors" )
	{
		std::uint8_t *sbuf = new std::uint8_t[ 4192 ];
		std::uint8_t *cbuf = new std::uint8_t[ 4192 ];
	
		ip::tcp::acceptor::ref	acceptor = new ip::tcp::acceptor( new ip::endpoint( 0 ) );
		REQUIRE( acceptor );
		
		acceptor->listen( 5 );
	
		acceptor->accept( [=]( int status, socket::ref sock ) mutable
		{
			REQUIRE( status == 0 );
			
			sock->add( tls::server::create() );
			
			sock->recv( [=]( int status, const std::uint8_t* sbuf, std::size_t len ) mutable
			{
				REQUIRE( status == 0 );
				
				sock->send( sbuf, len, [=]( int status )
				{
				} );
			} );
		} );
		
		ip::tcp::socket::ref	sock = new ip::tcp::socket;
		REQUIRE( sock );
		
		sock->add( tls::client::create() );
		
		sock->connect( new uri( "echo", "127.0.0.1", acceptor->endpoint()->port() ), [=]( int status, const endpoint::ref &peer ) mutable
		{
			REQUIRE( status == 0 );
			
			sock->send( ( const std::uint8_t* ) "Hello", 5, [=]( int status )
			{
				REQUIRE( status == 0 );
			} );
			
			sock->recv( [=]( int status, const std::uint8_t *buf, std::size_t len)
			{
				REQUIRE( status == 0 );
				REQUIRE( len == 5 );
				REQUIRE( buf[ 0 ] == 'H' );
				REQUIRE( buf[ 1 ] == 'e' );
				REQUIRE( buf[ 2 ] == 'l' );
				REQUIRE( buf[ 3 ] == 'l' );
				REQUIRE( buf[ 4 ] == 'o' );
				
				netkit::runloop::main()->stop();
			} );
		} );
		
		netkit::runloop::main()->run();
	}
}
