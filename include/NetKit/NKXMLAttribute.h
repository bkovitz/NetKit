#ifndef _coreapp_xml_attribute_h
#define _coreapp_xml_attribute_h

#include <CoreApp/CAXMLNode.h>

struct _xmlAttr;

namespace coreapp {
namespace xml {

class attribute : public node
{
public:

	typedef smart_ptr< attribute > ptr;

	attribute();
	
	attribute( _xmlAttr *attr );
	
	virtual ~attribute();
	
	attribute::ptr
	next() const;

	attribute::ptr
	prev() const;

protected:

	_xmlAttr *m_attr;
};

}

}

#endif
