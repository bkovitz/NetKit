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

TEST_CASE( "NetKit/uri/1", "netkit::uri tests" )
{
	netkit::uri::ref uri = new netkit::uri( "http://www.google.com:8080" );

	REQUIRE( uri->scheme() == "http" );
	REQUIRE( uri->host() == "www.google.com" );
	REQUIRE( uri->port() == 8080 );
	REQUIRE( uri->path().size() == 1 );
}


TEST_CASE( "NetKit/uri/2", "netkit::uri tests" )
{
	netkit::uri::ref uri = new netkit::uri( "/path?test=1" );

	REQUIRE( uri->scheme().size() == 0 );
	REQUIRE( uri->host().size() == 0 );
	REQUIRE( uri->path() == "/path" );
	REQUIRE( uri->query() == "test=1" );
}


TEST_CASE( "NetKit/uri/3", "netkit::uri tests" )
{
	netkit::uri::ref uri = new netkit::uri( "http://www.google.com/a%20space" );

	REQUIRE( uri->scheme() == "http" );
	REQUIRE( uri->host() == "www.google.com" );
	REQUIRE( uri->path() == "/a space" );
	REQUIRE( uri->query().size() == 0 );
}

TEST_CASE( "NetKit/uri/4", "netkit::uri tests" )
{
	netkit::uri::ref	uri = new netkit::uri( "http://www.google.com/path" );
	std::string		str = uri->to_string();

	REQUIRE( str == "http://www.google.com:80/path" );
}
