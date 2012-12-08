#define CATCH_CONFIG_RUNNER
#include <CoreApp/CoreApp.h>
#include "catch.hpp"

int main (int argc, char * const argv[])
{
	coreapp::log::set_level( coreapp::log::verbose );
    return Catch::Main( argc, argv );
}
