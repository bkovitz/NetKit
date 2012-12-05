#include "catch.hpp"
#include <CoreApp/CoreApp.h>
#if defined( __APPLE__ )
#	include <CoreFoundation/CoreFoundation.h>
#endif
#include <functional>

using namespace CoreApp;

TEST_CASE( "CoreApp/http/client", "http client tests" )
{
	http::client::ptr	client	= new http::client;
	http::request::ptr	request = new http::request( "http://www.porchdogsoft.com/test.html" );
	
	request->set_method( TEXT( "GET" ) );
	
	client->send( request, [&]( const http::response::ptr &response )
	{
		REQUIRE( response->status() == 200 );
		
		REQUIRE( response->body() == "<html>hello</html>\n" );
		
#if defined( __APPLE__ )

		CFRunLoopStop( CFRunLoopGetCurrent() );
#endif
	} );
	
#if defined( __APPLE__ )

	CFRunLoopRun();
	
#endif
}
