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

TEST_CASE( "NetKit/json/1", "json decoding tests" )
{
	netkit::json::value::ptr child;
	
	netkit::expected< netkit::json::value::ptr > ret = netkit::json::value::load( g_json );
	REQUIRE( ret.is_valid() );
	
	netkit::json::value::ptr root = ret.get();
	REQUIRE( root->is_object() );
	REQUIRE( root->is_member( "firstName" ) );
	
	netkit::json::value::ptr val = root[ "firstName" ];
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
	REQUIRE( val->as_integer() == 25 );
	
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
	REQUIRE( child->as_integer() == 10021 );
	
	val = root[ "phoneNumber" ];
	REQUIRE( val->is_array() );
	for ( int i = 0; i < val->size(); i++ )
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
	netkit::json::value::ptr	val;
	netkit::json::value::ptr	val2;
	netkit::json::value::ptr	arr;
	netkit::json::value::ptr	obj;
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
	REQUIRE( val[ "age" ]->as_integer() == 59 );
	
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
	
	std::string s = val->flatten();
	val2->load_from_string( s );
	
	REQUIRE( *val == *val2 );
}


class echo : public json::client
{
public:

	typedef std::function< void ( int32_t, const std::string&, int, double, const std::string& ) >	func_reply_f;
	typedef smart_ptr< echo >																		ptr;

	echo( const source::ptr &source )
	:
		client( source )
	{
	}
	
	void
	func( int i, double d, const std::string & s, func_reply_f reply ) const
	{
		json::value::ptr params;
		
		params[ "i" ] = i;
		params[ "d" ] = d;
		params[ "s" ] = s;
		
		( ( echo* ) this )->send_request( "func", params, [=]( int32_t error_code, const std::string &error_message, json::value::ptr result )
		{
			int			i	= 0;
			double		d	= 0;
			std::string s;
			
			if ( !error_code )
			{
				i = result[ "i" ]->as_integer();
				d = result[ "d" ]->as_real();
				s = result[ "s" ]->as_string();
			}
			
			reply( error_code, error_message, i, d, s );
		} );
	}
};


TEST_CASE( "NetKit/json/3", "json rpc" )
{
	tcp::server::ptr	sock_svr	= new tcp::server( new ip::address( inet_addr( "127.0.0.1" ), 0 ) );
	tcp::client::ptr	sock		= new tcp::client();
	echo::ptr			e			= new echo( sock );
	
	sock_svr->bind( { json::connection::adopt } );

	json::connection::bind( "func", [=]( json::value::ptr params, json::connection::reply_f reply )
	{
		int					i = params[ "i" ]->as_integer();
		double				d = params[ "d" ]->as_real();
		std::string			s = params[ "s" ]->as_string();
		json::value::ptr	response;
		json::value::ptr	result;
		
		result[ "i" ] = i;
		result[ "d" ] = d;
		result[ "s" ] = s;
		
		response[ "result" ] = result;
		
		reply( response );
	} );
	
	sock->connect( new ip::address( inet_addr( "127.0.0.1" ), sock_svr->port() ), [=]( int status )
	{
		REQUIRE( status == 0 );
		
		e->func( 7, 9.5, "hello world", [=]( int32_t error_code, const std::string &error_message, int i, double d, const std::string &s )
		{
			REQUIRE( error_code == 0 );
			REQUIRE( error_message.size() == 0 );
			REQUIRE( i == 7 );
			REQUIRE( d == 9.5 );
			REQUIRE( s == "hello world" );
			
			runloop::instance()->stop();
		} );
	} );
	
	runloop::instance()->run();
}