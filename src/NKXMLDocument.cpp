#include <NetKit/NKXMLDocument.h>
#include <libxml/tree.h>


using namespace netkit;
using namespace netkit::xml;


document::ptr
document::create( const std::string &str )
{
	document::ptr	self;
	xmlDocPtr		doc		= xmlReadMemory( str.c_str(), ( int ) str.size(), "noname.xml", NULL, 0 );
	
    if ( doc == NULL)
	{
		fprintf(stderr, "Failed to parse document\n");
		goto exit;
	}
	
	self = new document( doc );
	
exit:

	return self;
}


document::document()
:
	m_doc( NULL )
{
}


document::document( _xmlDoc *doc )
:
	m_doc( doc )
{
}


document::~document()
{
	if ( m_doc )
	{
		xmlFreeDoc( m_doc );
	}
}


node::ptr
document::root() const
{
	_xmlNode *node = xmlDocGetRootElement( m_doc );
	return ( node ) ? new xml::node( node ) : NULL;
}