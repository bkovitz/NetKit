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

#if defined( __APPLE__ )
#	include <CoreFoundation/CoreFoundation.h>
#endif

using namespace netkit;

TEST_CASE( "NetKit/ip/address/1", "ip::address tests" )
{
	ip::address::ptr addr = new ip::address( INADDR_ANY, 5000 );
	
	REQUIRE( addr->host() == "0.0.0.0" );
	REQUIRE( addr->port() == 5000 );
}


TEST_CASE( "NetKit/ip/address/2", "ip::address tests" )
{
	ip::address::ptr addr = new ip::address( inet_addr( "192.168.1.175" ), 9876 );
	
	REQUIRE( addr->host() == "192.168.1.175" );
	REQUIRE( addr->port() == 9876 );
}


TEST_CASE( "NetKit/ip/address/3", "ip::address tests" )
{
	ip::address::resolve( "www.google.com", 443, []( int status, ip::address::list addrs )
	{
		REQUIRE( status == 0 );
		REQUIRE( addrs.size() > 0 );
		
		runloop::instance()->stop();
	} );

	runloop::instance()->run();
}



