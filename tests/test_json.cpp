/*
 * Copyright (c) 2013, Porchdog Software Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of the FreeBSD Project.
 *
 */

#include "catch.hpp"
#include <NetKit/NetKit.h>

static const char * g_json = "{ \"firstName\": \"John\", \"lastName\": \"Smith\", \"age\": 25, \"human\" : true,\"address\": { \"streetAddress\": \"21 2nd Street\", \"city\": \"New York\", \"state\": \"NY\", \"postalCode\": 10021 }, \"phoneNumber\": [ { \"type\": \"home\", \"number\": \"212 555-1234\" }, { \"type\": \"fax\", \"number\": \"646 555-4567\" } ] }";

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
