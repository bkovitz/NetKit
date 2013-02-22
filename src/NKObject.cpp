#include <NetKit/NKObject.h>
#include <assert.h>

using namespace netkit;

object::~object()
{
	//assert( m_refs == 0 );
}
