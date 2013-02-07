#ifndef _coreapp_xml_element_h
#define _coreapp_xml_element_h

#include <CoreApp/CAXMLNode.h>
#include <string>

struct _xmlElement;

namespace coreapp {
namespace xml {


class element : public node
{
public:

	std::string
	version() const;
	
protected:

	_xmlElement *m_elem;
};

}

}

#endif
