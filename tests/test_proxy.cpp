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

TEST_CASE( "NetKit/proxy", "proxy tests" )
{
#if 0
	SECTION( "http", "http proxy" )
	{
		uri::ref uri		= new netkit::uri( "http://127.0.0.1:8080" );
		proxy::ref proxy	= new netkit::proxy( uri );
		
		proxy::set( proxy );
		
		proxy::on_auth_challenge( [=]() mutable
		{
			proxy->encode_authorization( "test", "test" );
			return true;
		} );
	
		http::request::ref request = new http::request( 1, 1, http::method::get, new netkit::uri( "http://www.apple.com" ) );
		request->add_to_header( "Accept", "*/*" );
		REQUIRE( request );
		
		request->on_reply( [=]( http::response::ref response )
		{
			REQUIRE( response );
			REQUIRE( response->status() == 200 );
			netkit::runloop::main()->stop();
		} );
		
		http::client::send( request );
	
		netkit::runloop::main()->run();
	}
	
	SECTION( "https", "https proxy" )
	{
		uri::ref uri		= new netkit::uri( "http://127.0.0.1:8080" );
		proxy::ref proxy	= new netkit::proxy( uri );
		
		proxy::set( proxy::null() );
		proxy::set( proxy );
		
		proxy::on_auth_challenge( [=]() mutable
		{
			proxy->encode_authorization( "test", "test" );
			return true;
		} );
	
		http::request::ref request = new http::request( 1, 1, http::method::get, new netkit::uri( "https://www.apple.com" ) );
		request->add_to_header( "Accept", "*/*" );
		REQUIRE( request );
		
		request->on_reply( [=]( http::response::ref response )
		{
			REQUIRE( response );
			REQUIRE( response->status() == 200 );
			netkit::runloop::main()->stop();
		} );
		
		http::client::send( request );
	
		netkit::runloop::main()->run();
	}
#endif
	SECTION( "socks5", "socks5 proxy" )
	{
		uri::ref uri		= new netkit::uri( "socks5://127.0.0.1:8080" );
		proxy::ref proxy	= new netkit::proxy( uri );
		
		proxy->encode_authorization( "test", "test" );
		
		proxy::set( proxy );
		
		http::request::ref request = new http::request( 1, 1, http::method::get, new netkit::uri( "http://www.apple.com" ) );
		request->add_to_header( "Accept", "*/*" );
		REQUIRE( request );
		
		request->on_reply( [=]( http::response::ref response )
		{
			REQUIRE( response );
			REQUIRE( response->status() == 200 );
			netkit::runloop::main()->stop();
		} );
		
		http::client::send( request );
	
		netkit::runloop::main()->run();
	}
	
	SECTION( "socks5", "socks5 proxy" )
	{
		uri::ref uri		= new netkit::uri( "socks5://127.0.0.1:8080" );
		proxy::ref proxy	= new netkit::proxy( uri );
	
		proxy->encode_authorization( "test", "test" );
		proxy::set( proxy );
		
		http::request::ref request = new http::request( 1, 1, http::method::get, new netkit::uri( "https://www.apple.com" ) );
		request->add_to_header( "Accept", "*/*" );
		REQUIRE( request );
		
		request->on_reply( [=]( http::response::ref response )
		{
			REQUIRE( response );
			REQUIRE( response->status() == 200 );
			netkit::runloop::main()->stop();
		} );
		
		http::client::send( request );
	
		netkit::runloop::main()->run();
	}
}
