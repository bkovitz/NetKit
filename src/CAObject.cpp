#include <CoreApp/object.h>
#include <assert.h>

using namespace CoreApp;

object::~object()
{
	assert( m_refs == 0 );
}
