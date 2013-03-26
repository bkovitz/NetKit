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

component::component()
:
	m_status( status::uninitialized )
{
	if ( !m_instances )
	{
		m_instances = new component::list;
	}
	
	m_instances->push_back( this );
}


component::~component()
{
}
