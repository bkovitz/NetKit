#define CATCH_CONFIG_RUNNER
#include <NetKit/NetKit.h>
#include "catch.hpp"

int main (int argc, char * const argv[])
{
	netkit::log::set_level( netkit::log::verbose );
    return Catch::Main( argc, argv );
}
