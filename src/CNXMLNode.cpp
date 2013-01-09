#include <CoreApp/CAXMLNode.h>
#include <CoreApp/CAXMLDocument.h>
#include <CoreApp/CAXMLAttribute.h>
#include <libxml/tree.h>


using namespace coreapp;
using namespace coreapp::xml;


node::node()
{
}


node::node( _xmlNode *node )
:
	m_node( node )
{
}


node::~node()
{
}


std::string
node::name() const
{
	std::string s;
	
	if ( m_node && m_node->name )
	{
		s = ( char* ) m_node->name;
	}
	
	return s;
}


node::ptr
node::parent() const
{
	return ( m_node && m_node->parent ) ? new node( m_node->parent ) : NULL;
}


node::ptr
node::children() const
{
	return ( m_node && m_node->children ) ? new node( m_node->children ) : NULL;
}


node::ptr
node::last() const
{
	return ( m_node && m_node->last ) ? new node( m_node->last ) : NULL;
}


node::ptr
node::next() const
{
	return ( m_node && m_node->next ) ? new node( m_node->next ) : NULL;
}


node::ptr
node::prev() const
{
	return ( m_node && m_node->prev ) ? new node( m_node->prev ) : NULL;
}


document_ptr
node::document() const
{
	return ( m_node && m_node->doc ) ? new xml::document( m_node->doc ) : NULL;
}


std::string
node::content() const
{
	std::string s;
	
	if ( m_node && m_node->content )
	{
		s = ( char* ) m_node->content;
	}
	
	return s;
}


attribute_ptr
node::attributes() const
{
	return ( m_node && m_node->properties ) ? new attribute( m_node->properties ) : NULL;
}