#include <CoreApp/CAObject.h>
#include <assert.h>

using namespace coreapp;

object::~object()
{
	assert( m_refs == 0 );
}
