#include "catch.hpp"
#include <CoreApp/ip_address.h>
#include <CoreApp/dispatch.h>
#if defined( __APPLE__ )
#	include <CoreFoundation/CoreFoundation.h>
#endif

using namespace CoreApp;

TEST_CASE( "CoreApp/ip/address/1", "ip::address tests" )
{
	ip::address::ptr addr = new ip::address( INADDR_ANY, 5000 );
	
	REQUIRE( addr->host() == "0.0.0.0" );
	REQUIRE( addr->port() == 5000 );
}


TEST_CASE( "CoreApp/ip/address/2", "ip::address tests" )
{
	ip::address::ptr addr = new ip::address( inet_addr( "192.168.1.175" ), 9876 );
	
	REQUIRE( addr->host() == "192.168.1.175" );
	REQUIRE( addr->port() == 9876 );
}


TEST_CASE( "CoreApp/ip/address/3", "ip::address tests" )
{
	CoreApp::ip::address::resolve( "www.google.com", 443, []( int status, CoreApp::ip::address::list addrs )
	{
		REQUIRE( status == 0 );
		REQUIRE( addrs.size() > 0 );
		
		CFRunLoopStop( CFRunLoopGetCurrent() );
	} );

	CFRunLoopRun();
}



