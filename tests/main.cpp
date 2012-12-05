#define CATCH_CONFIG_RUNNER
#include <CoreApp/CoreApp.h>
#include "catch.hpp"

int main (int argc, char * const argv[])
{
	CoreApp::log::set_level( CoreApp::log::verbose );
    return Catch::Main( argc, argv );
}
