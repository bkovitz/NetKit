#ifndef _netkit_xml_document_h
#define _netkit_xml_document_h

#include <NetKit/NKXMLNode.h>

struct _xmlDoc;

namespace netkit {
namespace xml {


class document : public node
{
public:

	typedef smart_ptr< document > ptr;

	static document::ptr
	create( const std::string &str );
	
	document();
	
	document( _xmlDoc *doc );
	
	virtual ~document();
	
	node::ptr
	root() const;
	
	std::string
	name() const;

	std::string
	version() const;
	
protected:

	_xmlDoc *m_doc;
};

}

}

#endif
