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
		
		response->add_to_header( "Content-Length", 5 );
		
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
		REQUIRE( response->status() == 200 );
	} );
	
	runloop::instance()->run();
}