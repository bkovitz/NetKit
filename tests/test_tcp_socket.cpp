#include "catch.hpp"
#include <CoreApp/tcp_socket.h>
#include <CoreApp/ip_address.h>

TEST_CASE( "CoreApp/socket", "socket tests" )
{
	SECTION( "constructors", "socket constructors" )
	{
		CoreApp::tcp::client::ptr sock = new CoreApp::tcp::client;

		REQUIRE( sock );
	}
}
