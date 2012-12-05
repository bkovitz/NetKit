#ifndef _CoreApp_xml_attribute_h
#define _CoreApp_xml_attribute_h

#include <CoreApp/xml/node.h>
#include <CoreApp/tstring.h>

struct _xmlAttr;

namespace CoreApp {
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
