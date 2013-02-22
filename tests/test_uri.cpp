#include "catch.hpp"
#include <NetKit/NetKit.h>

TEST_CASE( "NetKit/uri/1", "netkit::uri tests" )
{
	netkit::uri::ptr uri = new netkit::uri( "http://www.google.com:8080" );

	REQUIRE( uri->scheme() == "http" );
	REQUIRE( uri->host() == "www.google.com" );
	REQUIRE( uri->port() == 8080 );
	REQUIRE( uri->path().size() == 0 );
}


TEST_CASE( "NetKit/uri/2", "netkit::uri tests" )
{
	netkit::uri::ptr uri = new netkit::uri( "/path?test=1" );

	REQUIRE( uri->scheme().size() == 0 );
	REQUIRE( uri->host().size() == 0 );
	REQUIRE( uri->path() == "/path" );
	REQUIRE( uri->query() == "test=1" );
}


TEST_CASE( "NetKit/uri/3", "netkit::uri tests" )
{
	netkit::uri::ptr uri = new netkit::uri( "http://www.google.com/a%20space" );

	REQUIRE( uri->scheme() == "http" );
	REQUIRE( uri->host() == "www.google.com" );
	REQUIRE( uri->path() == "/a space" );
	REQUIRE( uri->query().size() == 0 );
}

TEST_CASE( "NetKit/uri/4", "netkit::uri tests" )
{
	netkit::uri::ptr	uri = new netkit::uri( "http://www.google.com/path" );
	std::string		str = uri->recompose();

	REQUIRE( str == "http://www.google.com:80/path" );
}
