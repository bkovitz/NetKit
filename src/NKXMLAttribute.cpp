#include <NetKit/NKXMLAttribute.h>
#include <libxml/tree.h>

using namespace netkit;
using namespace netkit::xml;


attribute::attribute()
{
}


attribute::attribute( _xmlAttr *attr )
:
	node( ( _xmlNode*) attr ),
	m_attr( attr )
{
}


attribute::~attribute()
{
}