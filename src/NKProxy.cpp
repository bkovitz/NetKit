#include <NetKit/NKProxy.h>

using namespace netkit;

proxy::proxy( uri::ptr uri )
:
	m_uri( uri ),
	m_port( 80 )
{
}


proxy::~proxy()
{
}