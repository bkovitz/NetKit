#include "catch.hpp"
#include <CoreApp/CoreApp.h>
#if defined( __APPLE__ )
#	include <CoreFoundation/CoreFoundation.h>
#endif

using namespace CoreApp;

class handler : public http::server::handler
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
		if ( request->uri()->path() == TEXT( "/found.html" ) )
		{
			http::response::ptr response = new http::response( 200 );
			
			conn->send( response );
		}
		
		return 0;
	}
};

TEST_CASE( "CoreApp/http/server/1", "http server tests" )
{
	tcp::server::ptr	sock_svr	= new tcp::server( new ip::address( inet_addr( "127.0.0.1" ), 0 ) );
	http::server::ptr	server		= new http::server;
	http::client::ptr	client		= new http::client;
	http::request::ptr	request;
	handler				handler;
	char				buf[ 1024 ];
	
	server->set_handler( "/found", &handler );
	sock_svr->add_listener( server.get() );
	
	sprintf( buf, "http://127.0.0.1:%d/notfound.html", sock_svr->port() );
	
	request	= new http::request( buf );
	
	request->set_method( TEXT( "GET" ) );
	
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


TEST_CASE( "CoreApp/http/server/2", "http server tests" )
{
	tcp::server::ptr	sock_svr	= new tcp::server( new ip::address( inet_addr( "127.0.0.1" ), 0 ) );
	http::server::ptr	server		= new http::server;
	http::client::ptr	client		= new http::client;
	http::request::ptr	request;
	handler				handler;
	char				buf[ 1024 ];
	
	server->set_handler( "/found", &handler );
	sock_svr->add_listener( server.get() );
	
	sprintf( buf, "http://127.0.0.1:%d/found.html", sock_svr->port() );
	
	request	= new http::request( buf );
	
	request->set_method( TEXT( "GET" ) );
	
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