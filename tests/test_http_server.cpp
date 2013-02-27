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

TEST_CASE( "NetKit/http/server/1", "http server tests" )
{
	tcp::server::ptr	sock_svr	= new tcp::server( new ip::address( inet_addr( "127.0.0.1" ), 0 ) );
	http::request::ptr	request;
	char				buf[ 1024 ];
	
	sock_svr->bind( { http::connection::adopt } );

	http::connection::bind( http::method::get, "/found", "*", [=]( http::request::ptr request, http::response_f func )
	{
		http::response::ptr response = new http::response( 200 );
		
		*response << "hello";
		
		func( response );
	} );
	
	sprintf( buf, "http://127.0.0.1:%d/found", sock_svr->port() );
	
	request	= new http::request( http::method::get, new uri( buf ) );
	
	http::client::send( request, [=]( int32_t error, const http::response::ptr &response )
	{
		REQUIRE( response->status() == 200 );
		
		runloop::instance()->stop();
	} );
	
	runloop::instance()->run();
}


TEST_CASE( "NetKit/http/server/2", "http server tests" )
{
	tcp::server::ptr	sock_svr	= new tcp::server( new ip::address( inet_addr( "127.0.0.1" ), 0 ) );
	http::request::ptr	request;
	char				buf[ 1024 ];
	
	sock_svr->bind( { http::connection::adopt } );
	
	http::connection::bind( http::method::get, "/found", "*", [=]( http::request::ptr request, http::response_f func )
	{
		http::response::ptr response = new http::response( 200 );
		
		func( response );
	} );
	
	sprintf( buf, "http://127.0.0.1:%d/notfound", sock_svr->port() );
	
	request	= new http::request( http::method::get, new uri( buf ) );
	
	http::client::send( request, [&]( int32_t error, const http::response::ptr &response )
	{
		REQUIRE( response->status() == 404 );
		runloop::instance()->stop();
	} );
	
	runloop::instance()->run();
}