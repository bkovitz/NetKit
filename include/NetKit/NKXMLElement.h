#ifndef _netkit_xml_element_h
#define _netkit_xml_element_h

#include <NetKit/NKXMLNode.h>
#include <string>

struct _xmlElement;

namespace netkit {
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
