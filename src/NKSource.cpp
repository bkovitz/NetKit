#include <NetKit/NKSource.h>

using namespace netkit;

source::source()
{
}


source::~source()
{
}


void
source::bind( sink::ptr sink )
{
	m_sink = sink;
}