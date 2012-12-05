#ifndef _CoreApp_xml_node_h
#define _CoreApp_xml_node_h

#include <CoreApp/object.h>
#include <CoreApp/tstring.h>

struct _xmlNode;

namespace CoreApp {
namespace xml {

class document;
typedef smart_ptr< document > document_ptr;

class attribute;
typedef smart_ptr< attribute > attribute_ptr;

class node : public object
{
public:

	typedef smart_ptr< node > ptr;

	node();
	
	node( _xmlNode *node );

	virtual ~node();

	std::tstring
	name() const;
	
	node::ptr
	parent() const;

	node::ptr
	children() const;

	node::ptr
	last() const;

	node::ptr
	next() const;

	node::ptr
	prev() const;

	document_ptr
	document() const;

	std::tstring
	content() const;

	attribute_ptr
	attributes() const;

protected:

	_xmlNode* m_node;
};

}

}

#endif
