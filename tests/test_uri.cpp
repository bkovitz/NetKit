#include "catch.hpp"
#include <CoreApp/CoreApp.h>

TEST_CASE( "CoreApp/uri/1", "CoreApp::uri tests" )
{
	CoreApp::uri::ptr uri = new CoreApp::uri( TEXT( "http://www.google.com:8080" ) );

	REQUIRE( uri->scheme() == TEXT( "http" ) );
	REQUIRE( uri->host() == TEXT( "www.google.com" ) );
	REQUIRE( uri->port() == 8080 );
	REQUIRE( uri->path().size() == 0 );
}


TEST_CASE( "CoreApp/uri/2", "CoreApp::uri tests" )
{
	CoreApp::uri::ptr uri = new CoreApp::uri( TEXT( "/path?test=1" ) );

	REQUIRE( uri->scheme().size() == 0 );
	REQUIRE( uri->host().size() == 0 );
	REQUIRE( uri->path() == TEXT( "/path" ) );
	REQUIRE( uri->query() == TEXT( "test=1" ) );
}


TEST_CASE( "CoreApp/uri/3", "CoreApp::uri tests" )
{
	CoreApp::uri::ptr uri = new CoreApp::uri( TEXT( "http://www.google.com/a%20space" ) );

	REQUIRE( uri->scheme() == TEXT( "http" ) );
	REQUIRE( uri->host() == TEXT( "www.google.com" ) );
	REQUIRE( uri->path() == TEXT( "/a space" ) );
	REQUIRE( uri->query().size() == 0 );
}

TEST_CASE( "CoreApp/uri/4", "CoreApp::uri tests" )
{
	CoreApp::uri::ptr	uri = new CoreApp::uri( TEXT( "http://www.google.com/path" ) );
	std::tstring		str = uri->recompose();

	REQUIRE( str == TEXT( "http://www.google.com:80/path" ) );
}
