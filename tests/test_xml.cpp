#include "catch.hpp"
#include <CoreApp/CoreApp.h>

static const char * g_xml = "<?xml version=\"1.0\"?><catalog><book id=\"bk101\"> <author>Gambardella, Matthew</author> <title>XML Developer's Guide</title> <genre>Computer</genre> <price>44.95</price> <publish_date>2000-10-01</publish_date> <description>An in-depth look at creating applications with XML.</description> </book> </catalog>";


TEST_CASE( "CoreApp/xml/1", "xml tests" )
{
	coreapp::xml::document::ptr document = coreapp::xml::document::create( g_xml );
	REQUIRE( document );
	
	coreapp::xml::node::ptr root = document->root();
	REQUIRE( root );
	REQUIRE( root->name() == "catalog" );
	
	coreapp::xml::node::ptr child = root->children();
	REQUIRE( child );
	REQUIRE( child->name() == "book" );
	
	coreapp::xml::attribute::ptr attr = child->attributes();
	REQUIRE( attr );
	REQUIRE( attr->name() == "id" );
}