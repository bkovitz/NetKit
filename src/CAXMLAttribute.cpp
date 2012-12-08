#include <CoreApp/CAXMLAttribute.h>
#include <libxml/tree.h>

using namespace coreapp;
using namespace coreapp::xml;


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