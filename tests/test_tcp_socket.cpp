#include "catch.hpp"
#include <NetKit/NetKit.h>

TEST_CASE( "NetKit/socket", "socket tests" )
{
	SECTION( "constructors", "socket constructors" )
	{
		netkit::tcp::client::ptr sock = new netkit::tcp::client;

		REQUIRE( sock );
	}
}
