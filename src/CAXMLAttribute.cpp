#include <CoreApp/xml/attribute.h>
#include <libxml/tree.h>

using namespace CoreApp;
using namespace CoreApp::xml;


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