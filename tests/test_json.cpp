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

using namespace netkit;

static const char * g_json = "{ \"firstName\": \"John\", \"lastName\": \"Smith\", \"age\": 25, \"human\" : true,\"address\": { \"streetAddress\": \"21 2nd Street\", \"city\": \"New York\", \"state\": \"NY\", \"postalCode\": 10021 }, \"phoneNumber\": [ { \"type\": \"home\", \"number\": \"212 555-1234\" }, { \"type\": \"fax\", \"number\": \"646 555-4567\" } ] }";

static std::uint8_t g_buf[ 64 ];
		
TEST_CASE( "NetKit/json/1", "json decoding tests" )
{
	netkit::json::value::ref child;
	
	netkit::expected< netkit::json::value::ref > ret = netkit::json::value::load( g_json );
	REQUIRE( ret.is_valid() );
	
	netkit::json::value::ref root = ret.get();
	REQUIRE( root->is_object() );
	REQUIRE( root->is_member( "firstName" ) );
	
	netkit::json::value::ref val = root[ "firstName" ];
	REQUIRE( val->is_string() );
	REQUIRE( val->as_string() == "John" );
	REQUIRE( *val != "James" );
	REQUIRE( *val != 9 );
	REQUIRE( *val != 7.5 );
	REQUIRE( *val == "John" );
	
	val = root[ "lastName" ];
	REQUIRE( val->is_string() );
	REQUIRE( val->as_string() == "Smith" );
	
	val = root[ "age" ];
	REQUIRE( val->is_integer() );
	REQUIRE( val->as_int32() == 25 );
	
	val = root[ "human" ];
	REQUIRE( val->is_bool() );
	REQUIRE( val->as_bool() == true );
	
	val = root[ "address" ];
	REQUIRE( val->is_object() );
	
	child = val[ "streetAddress" ];
	REQUIRE( child->is_string() );
	REQUIRE( child->as_string() == "21 2nd Street" );
	
	child = val[ "city" ];
	REQUIRE( child->is_string() );
	REQUIRE( child->as_string() == "New York" );
	
	child = val[ "state" ];
	REQUIRE( child->is_string() );
	REQUIRE( child->as_string() == "NY" );
	
	child = val[ "postalCode" ];
	REQUIRE( child->is_integer() );
	REQUIRE( child->as_int32() == 10021 );
	
	val = root[ "phoneNumber" ];
	REQUIRE( val->is_array() );
	for ( std::size_t i = 0; i < val->size(); i++ )
	{
		child = val[ i ];
		REQUIRE( child->is_object() );
		
		REQUIRE( child[ "type" ]->is_string() );
		REQUIRE( child[ "number" ]->is_string() );
		REQUIRE( child[ "age" ]->is_null() );
	}
	
	val = root[ "im" ];
	REQUIRE( val->is_null() );
}


TEST_CASE( "NetKit/json/2", "json encoding tests" )
{
	netkit::json::value::ref	val;
	netkit::json::value::ref	val2;
	netkit::json::value::ref	arr;
	netkit::json::value::ref	obj;
	bool						ok;
	
	REQUIRE( !val[ "not_there" ]->is_string() );
	
	val[ "firstName" ] = "John";
	val[ "firstName" ]->is_string();
	REQUIRE( val[ "firstName" ]->as_string() == "John" );
	
	val[ "lastName" ] = "Doe";
	REQUIRE( val[ "lastName" ]->is_string() );
	REQUIRE( val[ "lastName" ]->as_string() == "Doe" );
	
	val[ "age" ] = 59;
	REQUIRE( val[ "age" ]->is_integer() );
	REQUIRE( val[ "age" ]->as_int32() == 59 );
	
	val[ "inner" ][ "firstName" ] = "Exene";
	REQUIRE( val[ "inner" ][ "firstName" ]->is_string() );
	REQUIRE( val[ "inner" ][ "firstName" ]->as_string() == "Exene" );
	
	val[ "inner" ][ "lastName" ] = "Cervenka";
	REQUIRE( val[ "inner" ][ "lastName" ]->is_string() );
	REQUIRE( val[ "inner" ][ "lastName" ]->as_string() == "Cervenka" );
	
	ok = arr->append( 7 );
	REQUIRE( ok );
	ok = arr->append( 8 );
	REQUIRE( ok );
	ok = arr->append( 9 );
	REQUIRE( ok );
	
	val[ "numbers" ] = arr;
	
	std::string s = val->to_string();
	val2->load_from_string( s );
	
	REQUIRE( *val == *val2 );
}


class echo : public json::client
{
public:

	typedef std::function< void ( netkit::status, const std::string&, int, double, const std::string& ) >	func_reply_f;
	typedef smart_ref< echo >																				ref;

	echo()
	{
	}
	
	void
	func( int i, double d, const std::string & s, func_reply_f reply )
	{
		json::value::ref params;
		
		params[ "i" ] = i;
		params[ "d" ] = d;
		params[ "s" ] = s;
		
		send_request( "func", params, [=]( netkit::status error_code, const std::string &error_message, json::value::ref result )
		{
			int			i	= 0;
			double		d	= 0;
			std::string s;
			
			if ( error_code == netkit::status::ok )
			{
				i = result[ "i" ]->as_int32();
				d = result[ "d" ]->as_real();
				s = result[ "s" ]->as_string();
			}
			
			reply( error_code, error_message, i, d, s );
		} );
	}
};


TEST_CASE( "NetKit/json/3", "json rpc" )
{
	ip::tcp::acceptor::ref	acceptor	= new ip::tcp::acceptor( new ip::endpoint( AF_INET, 0 ) );
	echo::ref				e			= new echo;
	
	acceptor->accept( [=]( int status, socket::ref sock )
	{
		REQUIRE( status == 0 );
		netkit::json::connection::ref conn = new netkit::json::connection;
		conn->bind( sock.get() );
	} );

	json::server::bind( "func", 3, ( json::server::request_f ) [=]( json::value::ref params, json::server::reply_f reply )
	{
		int					i = params[ "i" ]->as_int32();
		double				d = params[ "d" ]->as_real();
		std::string			s = params[ "s" ]->as_string();
		json::value::ref	response;
		json::value::ref	result;
		
		result[ "i" ] = i;
		result[ "d" ] = d;
		result[ "s" ] = s;
		
		response[ "result" ] = result;
		
		reply( response, false );
	} );
	
	e->connect( new uri( "json", "127.0.0.1", acceptor->endpoint()->port() ), [=]( int status, const endpoint::ref &peer ) mutable
	{
		REQUIRE( status == 0 );
		
		e->func( 7, 9.5, "hello world", [=]( netkit::status error_code, const std::string &error_message, int i, double d, const std::string &s )
		{
			//REQUIRE( error_code == netkit::status::ok );
			REQUIRE( error_message.size() == 0 );
			REQUIRE( i == 7 );
			REQUIRE( d == 9.5 );
			REQUIRE( s == "hello world" );
			
			runloop::main()->stop();
		} );
	} );
	
	runloop::main()->run();
}
