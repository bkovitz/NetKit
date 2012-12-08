#include "catch.hpp"
#include <CoreApp/CoreApp.h>

TEST_CASE( "CoreApp/uri/1", "coreapp::uri tests" )
{
	coreapp::uri::ptr uri = new coreapp::uri( "http://www.google.com:8080" );

	REQUIRE( uri->scheme() == "http" );
	REQUIRE( uri->host() == "www.google.com" );
	REQUIRE( uri->port() == 8080 );
	REQUIRE( uri->path().size() == 0 );
}


TEST_CASE( "CoreApp/uri/2", "coreapp::uri tests" )
{
	coreapp::uri::ptr uri = new coreapp::uri( "/path?test=1" );

	REQUIRE( uri->scheme().size() == 0 );
	REQUIRE( uri->host().size() == 0 );
	REQUIRE( uri->path() == "/path" );
	REQUIRE( uri->query() == "test=1" );
}


TEST_CASE( "CoreApp/uri/3", "coreapp::uri tests" )
{
	coreapp::uri::ptr uri = new coreapp::uri( "http://www.google.com/a%20space" );

	REQUIRE( uri->scheme() == "http" );
	REQUIRE( uri->host() == "www.google.com" );
	REQUIRE( uri->path() == "/a space" );
	REQUIRE( uri->query().size() == 0 );
}

TEST_CASE( "CoreApp/uri/4", "coreapp::uri tests" )
{
	coreapp::uri::ptr	uri = new coreapp::uri( "http://www.google.com/path" );
	std::string		str = uri->recompose();

	REQUIRE( str == "http://www.google.com:80/path" );
}
