#include "catch.hpp"
#include <CoreApp/CATCPSocket.h>
#include <CoreApp/CAIPAddress.h>

TEST_CASE( "CoreApp/socket", "socket tests" )
{
	SECTION( "constructors", "socket constructors" )
	{
		coreapp::tcp::client::ptr sock = new coreapp::tcp::client;

		REQUIRE( sock );
	}
}
