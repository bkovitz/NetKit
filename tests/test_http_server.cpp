#include "catch.hpp"
#include <NetKit/NetKit.h>
#if defined( __APPLE__ )
#	include <CoreFoundation/CoreFoundation.h>
#endif

using namespace netkit;

class handler : public http::service::handler
{
	virtual http::request::ptr
	message_will_begin( const uri::ptr &uri )
	{
		http::request::ptr request( new http::request( uri ) );
		request->set_context( this );
		return request;
	}
	
	virtual int
	headers_were_received( http::connection::ptr conn, http::request::ptr &request )
	{
		return 0;
	}
		
	virtual int
	message_was_received( http::connection::ptr conn, http::request::ptr &request )
	{
		if ( request->uri()->path() == "/found.html" )
		{
			http::response::ptr response = new http::response( 200 );
			
			conn->send( response );
		}
		
		return 0;
	}
};

TEST_CASE( "NetKit/http/server/1", "http server tests" )
{
	tcp::server::ptr	sock_svr	= new tcp::server( new ip::address( inet_addr( "127.0.0.1" ), 0 ) );
	http::service::ptr	service		= new http::service;
	http::client::ptr	client		= new http::client;
	http::request::ptr	request;
	handler				handler;
	char				buf[ 1024 ];
	
	service->set_handler( "/found", &handler );
	sock_svr->add_service( service.get() );
	
	sprintf( buf, "http://127.0.0.1:%d/notfound.html", sock_svr->port() );
	
	request	= new http::request( buf );
	
	request->set_method( "GET" );
	
	client->send( request, [&]( const http::response::ptr &response )
	{
		REQUIRE( response->status() == 404 );
		
#if defined( __APPLE__ )

		CFRunLoopStop( CFRunLoopGetCurrent() );

#endif
	} );
	
#if defined( __APPLE__ )

	CFRunLoopRun();
	
#endif
}


TEST_CASE( "NetKit/http/server/2", "http server tests" )
{
	tcp::server::ptr	sock_svr	= new tcp::server( new ip::address( inet_addr( "127.0.0.1" ), 0 ) );
	http::service::ptr	service		= new http::service;
	http::client::ptr	client		= new http::client;
	http::request::ptr	request;
	handler				handler;
	char				buf[ 1024 ];
	
	service->set_handler( "/found", &handler );
	sock_svr->add_service( service.get() );
	
	sprintf( buf, "http://127.0.0.1:%d/found.html", sock_svr->port() );
	
	request	= new http::request( buf );
	
	request->set_method( "GET" );
	
	client->send( request, [&]( const http::response::ptr &response )
	{
		REQUIRE( response->status() == 200 );
	
#if defined( __APPLE__ )

		CFRunLoopStop( CFRunLoopGetCurrent() );

#endif
	} );
	
#if defined( __APPLE__ )

	CFRunLoopRun();
	
#endif
}