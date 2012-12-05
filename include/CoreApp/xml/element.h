#ifndef _CoreApp_xml_element_h
#define _CoreApp_xml_element_h

#include <CoreApp/xml/node.h>
#include <CoreApp/tstring.h>

struct _xmlElement;

namespace CoreApp {
namespace xml {


class element : public node
{
public:

	std::tstring
	version() const;
	
protected:

	_xmlElement *m_elem;
};

}

}

#endif
