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
#include <sstream>

using namespace netkit;

TEST_CASE( "NetKit/http/server/1", "http server tests" )
{
	ip::tcp::acceptor::ref	acceptor	= new ip::tcp::acceptor( new ip::endpoint( 0 ) );
	http::request::ref		request;
	std::ostringstream		os;
	
	acceptor->accept( [=]( int status, socket::ref sock )
	{
		REQUIRE( status == 0 );
		sink::ref sink = http::server::adopt( sock.get() );
		REQUIRE( sink );
		sink->bind( sock.get() );
	} );
	
	http::server::bind( http::method::get, "/found", "*", [=]( http::request::ref request, http::server::response_f reply )
	{
		http::response::ref response = new http::response( request->major(), request->minor(), http::status::ok, false );
		
		response->add_to_header( "Content-Type", "text/plain" );
		response->add_to_header( "Content-Length", 5 );
		
		*response << "hello";
		
		reply( response, false );
		
		return 0;
	} );
	
	os << "http://127.0.0.1:" << acceptor->endpoint()->port() << "/found";
	
	request	= new http::request( http::method::get, 1, 1, new uri( os.str() ) );
	
	request->on_reply( [=]( http::response::ref response )
	{
		REQUIRE( response->status() == 200 );
		
		runloop::main()->stop();
	} );
	
	http::client::send( request );
	
	runloop::main()->run();
}


TEST_CASE( "NetKit/http/server/2", "http server tests" )
{
	ip::tcp::acceptor::ref	acceptor	= new ip::tcp::acceptor( new ip::endpoint( 0 ) );
	http::request::ref		request;
	std::ostringstream		os;
	
	acceptor->accept( [=]( int status, socket::ref sock )
	{
		REQUIRE( status == 0 );
		sink::ref sink = http::server::adopt( sock.get() );
		REQUIRE( sink );
		sink->bind( sock.get() );
	} );
	
	http::server::bind( http::method::get, "/found", "*", [=]( http::request::ref request, http::server::response_f func )
	{
		http::response::ref response = new http::response( request->major(), request->minor(), http::status::ok, false );
		
		func( response, false );
		
		return 0;
	} );
	
	os << "http://127.0.0.1:" << acceptor->endpoint()->port() << "/notfound";
	
	request	= new http::request( http::method::get, 1, 1, new uri( os.str() ) );
	
	request->on_reply( [=]( http::response::ref response )
	{
		REQUIRE( response->status() == 404 );
		runloop::main()->stop();
	} );
	
	http::client::send( request );
	
	runloop::main()->run();
}


TEST_CASE( "NetKit/http/server/3", "http redirect tests" )
{
	http::request::ref request = new http::request( http::method::get, 1, 1, new uri( "http://www.apple.com/bonjour" ) );
	
	request->on_reply( [=]( http::response::ref response )
	{
		REQUIRE( response->status() == 200 );
		runloop::main()->stop();
	} );
	
	http::client::send( request );
	
	runloop::main()->run();
}


static void
do_accept( ip::tcp::acceptor::ref acceptor )
{
	static int i = 0;

	acceptor->accept( [=]( int status, socket::ref sock ) mutable
	{
nklog( log::verbose, "accepted %d connections\n", ++i );

		sink::ref sink = http::server::adopt( sock.get() );
		sink->bind( sock.get() );

		do_accept( acceptor );
	} );
}


TEST_CASE( "NetKit/http/server/4", "http stress tests" )
{
	ip::tcp::acceptor::ref	acceptor		= new ip::tcp::acceptor( new ip::endpoint( 0 ) );
	std::int32_t			max_count		= 500;
	std::int32_t			*messages_rcvd	= new std::int32_t;
	std::int32_t			*errors			= new std::int32_t;
	
	do_accept( acceptor );
	
	http::server::bind( http::method::get, "/found", "*", [=]( http::request::ref request, http::server::response_f reply )
	{
		http::response::ref response = new http::response( request->major(), request->minor(), http::status::ok, false );
		
		response->add_to_header( "Content-Type", "text/plain" );
		response->add_to_header( "Content-Length", 5 );
		
		*response << "hello";
		
		reply( response, false );
		
		return 0;
	} );

	*messages_rcvd	= 0;

	for ( int i = 0; i < max_count; i++ )
	{
		http::request::ref	request;
		std::ostringstream	os;

		os << "http://127.0.0.1:" << acceptor->endpoint()->port() << "/found";
	
		request	= new http::request( http::method::get, 1, 1, new uri( os.str() ) );
	
		request->on_reply( [=]( http::response::ref response )
		{
			REQUIRE( response->status() == 200 );

			*messages_rcvd = *messages_rcvd + 1;

			if ( *messages_rcvd == max_count )
			{
				runloop::main()->stop();
			}
		} );
		
		http::client::send( request );

		if ( ( i % 133 ) == 0 )
		{
			runloop::main()->run( runloop::mode::once );
		}
	}

	runloop::main()->run();
}
