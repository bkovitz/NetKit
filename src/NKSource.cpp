#include <NetKit/NKSource.h>

using namespace netkit;

source::source()
{
}


source::~source()
{
}


sink::ptr&
source::sink()
{
	return m_sink;
}


const sink::ptr&
source::sink() const
{
	return m_sink;
}


void
source::bind( sink::ptr sink )
{
	m_sink = sink;
}