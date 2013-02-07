#include "catch.hpp"
#include <CoreNetwork/CoreNetwork.h>

static const char * g_json = "{ \"firstName\": \"John\", \"lastName\": \"Smith\", \"age\": 25, \"human\" : true,\"address\": { \"streetAddress\": \"21 2nd Street\", \"city\": \"New York\", \"state\": \"NY\", \"postalCode\": 10021 }, \"phoneNumber\": [ { \"type\": \"home\", \"number\": \"212 555-1234\" }, { \"type\": \"fax\", \"number\": \"646 555-4567\" } ] }";

/*
	{
		"firstName": "John",
		"lastName": "Smith",
		"age": 25,
		"address":
		{
			"streetAddress": "21 2nd Street",
			"city": "New York",
			"state": "NY",
			"postalCode": 10021
		},
		"phoneNumber":
		[
			{
				"type": "home",
				"number": "212 555-1234"
			},
			{
				"type": "fax",
				"number": "646 555-4567"
			}
		]
	}
*/

TEST_CASE( "NetKit/json/1", "json decoding tests" )
{
	netkit::json::value::ptr child;
	
	netkit::expected< netkit::json::value::ptr > ret = netkit::json::value::load( g_json );
	REQUIRE( ret.is_valid() );
	
	netkit::json::value::ptr root = ret.get();
	REQUIRE( root->is_object() );
	REQUIRE( root->is_member( "firstName" ) );
	
	netkit::json::value::ptr val = ( *root )[ "firstName" ];
	REQUIRE( val->is_string() );
	REQUIRE( val->as_string().get() == "John" );
	
	val = ( *root )[ "lastName" ];
	REQUIRE( val->is_string() );
	REQUIRE( val->as_string().get() == "Smith" );
	
	val = ( *root )[ "age" ];
	REQUIRE( val->is_integer() );
	REQUIRE( val->as_int().get() == 25 );
	
	val = ( *root )[ "human" ];
	REQUIRE( val->is_bool() );
	REQUIRE( val->as_bool().get() == true );
	
	val = ( *root )[ "address" ];
	REQUIRE( val->is_object() );
	
	child = ( *val )[ "streetAddress" ];
	REQUIRE( child->is_string() );
	REQUIRE( child->as_string().get() == "21 2nd Street" );
	
	child = ( *val )[ "city" ];
	REQUIRE( child->is_string() );
	REQUIRE( child->as_string().get() == "New York" );
	
	child = ( *val )[ "state" ];
	REQUIRE( child->is_string() );
	REQUIRE( child->as_string().get() == "NY" );
	
	child = ( *val )[ "postalCode" ];
	REQUIRE( child->is_integer() );
	REQUIRE( child->as_int().get() == 10021 );
	
	val = ( *root )[ "phoneNumber" ];
	REQUIRE( val->is_array() );
	for ( int i = 0; i < val->size(); i++ )
	{
		child = ( *val )[ i ];
		REQUIRE( child->is_object() );
		
		REQUIRE( ( *child )[ "type" ]->is_string() );
		REQUIRE( ( *child )[ "number" ]->is_string() );
		REQUIRE( ( *child )[ "age" ]->is_null() );
	}
	
	val = ( *root )[ "im" ];
	REQUIRE( val->is_null() );
}


TEST_CASE( "NetKit/json/2", "json encoding tests" )
{
	netkit::json::value::ptr	val = netkit::json::value::object();
	netkit::json::value::ptr	arr;
	netkit::json::value::ptr	obj;
	bool						ok;
	
	ok = val->set( "firstName", netkit::json::value::string( "John" ) );
	REQUIRE( ok );
	
	ok = val->set( "lastName", netkit::json::value::string( "Doe" ) );
	REQUIRE( ok );
	
	obj = netkit::json::value::object();
	ok = obj->set( "city", netkit::json::value::string( "los angeles" ) );
	REQUIRE( ok );
	
	ok = val->set( "map", obj );
	REQUIRE( ok );
	
	arr = netkit::json::value::array();
	ok = arr->append( netkit::json::value::integer( 7 ) );
	REQUIRE( ok );
	ok = arr->append( netkit::json::value::integer( 8 ) );
	REQUIRE( ok );
	ok = arr->append( netkit::json::value::integer( 9 ) );
	REQUIRE( ok );
	
	ok = val->set( "numbers", arr );
	REQUIRE( ok );
	
	std::string s = val->flatten( netkit::json::value::flatten_flag_preserve_order );
	fprintf( stderr, "s = %s\n", s.c_str() );
}
