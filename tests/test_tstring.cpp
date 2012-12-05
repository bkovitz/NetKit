#include "catch.hpp"
#include <CoreApp/CoreApp.h>

TEST_CASE( "CoreApp/tstring/1", "tstring tests" )
{
	std::tstring t = TEXT( "hello" );

	REQUIRE( t == "hello" );
	REQUIRE( t.utf8() == "hello" );
}


TEST_CASE( "CoreApp/tstring/2", "tstring tests" )
{
	std::tstring t1 = TEXT( "hello" );
	std::tstring t2 = TEXT( " world" );
	std::tstring t3 = t1 + t2;

	REQUIRE( t3 == TEXT( "hello world" ) );
}



