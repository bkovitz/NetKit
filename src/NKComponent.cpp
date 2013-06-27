#include <NetKit/NKComponent.h>
#include <NetKit/NKError.h>

using namespace netkit;

#if defined( __APPLE__ )
#	pragma mark component implementation
#endif

// -----------------------------
// component implementation
// -----------------------------

component::list *component::m_instances;

bool
component::initialize( const std::vector< std::string > &command_line )
{
	bool ok = true;
	
	for ( auto it = component::begin(); it != component::end(); it++ )
	{
		if ( ( *it )->will_initialize( command_line ) != netkit::status::ok )
		{
			ok = false;
		}
	}

	for ( auto it = component::begin(); it != component::end(); it++ )
	{
		if ( ( *it )->did_initialize() != netkit::status::ok )
		{
			ok = false;
		}
	}
	
	return ok;
}


void
component::finalize()
{
	for ( auto it = component::begin(); it != component::end(); it++ )
	{
		( *it )->will_terminate();
	}
}


component::component()
:
	m_status( netkit::status::uninitialized )
{
	m_instances->push_back( this );
}


component::~component()
{
}
