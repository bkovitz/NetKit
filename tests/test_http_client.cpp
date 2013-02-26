#include "catch.hpp"
#include <NetKit/NetKit.h>
#include <functional>

using namespace netkit;

TEST_CASE( "NetKit/http/client", "http client tests" )
{
	http::request::ptr	request = new http::request( http::method::get, new uri( "http:://wwww.porchdogsoft.com/test.html" ) );
	
	http::client::send( request, [=]( uint32_t error, http::response::ptr response )
	{
		REQUIRE( response->status() == 200 );
		
		REQUIRE( response->body() == "<html>hello</html>\n" );
		
		runloop::instance()->stop();
	} );
	
	runloop::instance()->run();
}
