#include "catch.hpp"
#include <NetKit/NetKit.h>

static const char * g_xml = "<?xml version=\"1.0\"?><catalog><book id=\"bk101\"> <author>Gambardella, Matthew</author> <title>XML Developer's Guide</title> <genre>Computer</genre> <price>44.95</price> <publish_date>2000-10-01</publish_date> <description>An in-depth look at creating applications with XML.</description> </book> </catalog>";


TEST_CASE( "NetKit/xml/1", "xml tests" )
{
	netkit::xml::document::ptr document = netkit::xml::document::create( g_xml );
	REQUIRE( document );
	
	netkit::xml::node::ptr root = document->root();
	REQUIRE( root );
	REQUIRE( root->name() == "catalog" );
	
	netkit::xml::node::ptr child = root->children();
	REQUIRE( child );
	REQUIRE( child->name() == "book" );
	
	netkit::xml::attribute::ptr attr = child->attributes();
	REQUIRE( attr );
	REQUIRE( attr->name() == "id" );
}