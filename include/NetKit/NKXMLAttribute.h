#ifndef _netkit_xml_attribute_h
#define _netkit_xml_attribute_h

#include <NetKit/NKXMLNode.h>

struct _xmlAttr;

namespace netkit {
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
