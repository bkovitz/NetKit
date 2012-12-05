#ifndef _CoreApp_xml_document_h
#define _CoreApp_xml_document_h

#include <CoreApp/xml/node.h>
#include <CoreApp/tstring.h>

struct _xmlDoc;

namespace CoreApp {
namespace xml {


class document : public node
{
public:

	typedef smart_ptr< document > ptr;

	static document::ptr
	create( const std::tstring &str );
	
	document();
	
	document( _xmlDoc *doc );
	
	virtual ~document();
	
	node::ptr
	root() const;
	
	std::tstring
	name() const;

	std::tstring
	version() const;
	
protected:

	_xmlDoc *m_doc;
};

}

}

#endif
