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
 
#include "NKHTTP_Mac.h"
#include <NetKit/NKLog.h>

using namespace netkit::http;

request::ref
request::create( std::uint16_t major, std::uint16_t minor, int method, const uri::ref &uri )
{
	return new request_mac( major, minor, method, uri );
}


response::ref
response::create( std::uint16_t major, std::uint16_t minor, std::uint16_t status, bool keep_alive )
{
	return new response_mac( major, minor, status, keep_alive );
}


#if defined( __APPLE__ )
#	pragma mark client_mac implementation
#endif

client_mac::client_mac( const request::ref &request, auth_f auth_func, response_f response_func )
:
	client( request, auth_func, response_func ),
	m_stream( NULL )
{
	m_response = new response_mac( request->major(), request->minor(), http::status::ok, request->keep_alive() );
}


client_mac::~client_mac()
{
	close_stream();
}


void
client::send( const request::ref &request, response_f response_func )
{
	client_mac *self = new client_mac( request, [=]( request::ref &request, uint32_t status )
	{
		return false;
	}, response_func );

	self->send_request( NULL );
}



void
client::send( const request::ref &request, auth_f auth_func, response_f response_func )
{
	client_mac *self = new client_mac( request, auth_func, response_func );

	self->send_request( NULL );
}


void
client_mac::event_callback( CFReadStreamRef stream, CFStreamEventType event, void *context )
{
	client_mac *self = reinterpret_cast< client_mac* >( context );
	
	nklog( log::verbose, "in event_callback: event = %d", event );

	switch ( event )
	{
		case kCFStreamEventHasBytesAvailable:
		{
			UInt8	buf[ 1024 ];
			CFIndex num;
			
			num = CFReadStreamRead( stream, buf, sizeof( buf ) );
			
			if ( num )
			{
				self->m_response->write( buf, num );
			}
			else if ( num < 0 )
			{
				nklog( log::error, "CFReadStreamRead() failed" );
				self->reply( -1 );
				goto exit;
			}
		}
		break;

		case kCFStreamEventErrorOccurred:
		{
			nklog( log::error, "received kCFStreamEventErrorOccurred" );
			self->reply( -1 );
			goto exit;
		}
		break;

		case kCFStreamEventEndEncountered:
		{
			CFHTTPMessageRef header = ( CFHTTPMessageRef ) CFReadStreamCopyProperty( stream, kCFStreamPropertyHTTPResponseHeader );
			
			if ( !header )
			{
				nklog( log::error, "CFReadStreamCopyProperty( kCFStreamPropertyHTTPResponseHeader )() failed" );
				self->reply( -1 );
				goto exit;
			}
			
			self->m_response->set_status( ( int ) CFHTTPMessageGetResponseStatusCode( header ) );
			
			if ( ( self->m_response->status() == 401 ) || ( self->m_response->status() == 407 ) )
			{
				CFHTTPAuthenticationRef auth = CFHTTPAuthenticationCreateFromResponse( kCFAllocatorDefault, header );
				
				if ( !auth )
				{
					nklog( log::error, "CFHTTPAuthenticationCreateFromResponse() failed" );
					self->reply( -1 );
					goto exit;
				}
				
				if ( ( self->m_request->tries() == 1 ) && ( self->m_request->proxy()->user().length() > 0 ) )
				{
					self->send_request( auth );
				}
				else if ( self->m_auth_func( self->m_request, self->m_response->status() ) )
				{
					self->send_request( auth );
				}
				else
				{
					self->reply( 0 );
				}
				
				CFRelease( auth );
			}
			else
			{
				self->reply( 0 );
			}
			
			CFRelease( header );
		}
		break;
	}
	
exit:

	return;
}


void
client_mac::send_request( CFHTTPAuthenticationRef auth )
{
	CFStreamClientContext	context	= { 0, this, nil, nil, nil };
	CFHTTPMessageRef		handle	= CFHTTPMessageCreateRequest( kCFAllocatorDefault, ( CFStringRef ) [ NSString stringWithUTF8String:http::method::to_string( m_request->method() ).c_str() ], ( CFURLRef ) [ NSURL URLWithString:[ NSString stringWithUTF8String:m_request->uri()->to_string().c_str() ] ], kCFHTTPVersion1_1 );
	
	if ( !handle )
	{
		nklog( log::error, "CFHTTPMessageCreateRequest() failed" );
		reply( -1 );
		goto exit;
	}
	
	CFHTTPMessageSetHeaderFieldValue( handle, CFSTR( "Host" ), ( CFStringRef ) [ NSString stringWithFormat:@"%s:%d", m_request->uri()->host().c_str(), m_request->uri()->port() ] );
 
	for ( message::header::const_iterator it = m_request->heder().begin(); it != m_request->heder().end(); it++ )
	{
		CFHTTPMessageSetHeaderFieldValue( handle, ( CFStringRef ) [ NSString stringWithUTF8String:it->first.c_str() ], ( CFStringRef ) [ NSString stringWithUTF8String:it->second.c_str() ] );
	}
	
	if ( m_request->body().size() > 0 )
	{
		CFHTTPMessageSetHeaderFieldValue( handle, CFSTR( "Content-Length" ), ( CFStringRef ) [ NSString stringWithFormat:@"%ld", m_request->body().size() ] );
		CFHTTPMessageSetBody( handle, ( CFDataRef ) [ NSData dataWithBytesNoCopy:( void* ) &m_request->body()[ 0 ] length:m_request->body().size() freeWhenDone:NO ] );
	}
	
	if ( auth && m_request->proxy()->user().length() && m_request->proxy()->password().length() )
	{
		CFStreamError err;
			
		if ( !CFHTTPMessageApplyCredentials( handle, auth, ( CFStringRef ) [ NSString stringWithUTF8String:m_request->proxy()->user().c_str() ], ( CFStringRef ) [ NSString stringWithUTF8String:m_request->proxy()->password().c_str() ], &err ) )
		{
			nklog( log::error, "CFHTTPMessageApplyCredentials() failed: %d", err.error );
			reply( err.error );
			goto exit;
		}
	}
	
	if ( m_stream )
	{
		close_stream();
	}
	
	m_stream = CFReadStreamCreateForHTTPRequest( kCFAllocatorDefault, handle );
	
	if ( !m_stream )
	{
		nklog( log::error, "CFReadStreamCreateForHTTPRequest() failed" );
		reply( -1 );
		goto exit;
	}
	
	CFReadStreamSetProperty( m_stream, kCFStreamPropertyHTTPShouldAutoredirect, kCFBooleanTrue);
	
	CFReadStreamSetProperty( m_stream, kCFStreamPropertyHTTPAttemptPersistentConnection, kCFBooleanTrue );
	
	if ( m_request->proxy() )
	{
		NSDictionary *dict;
								
		if ( m_request->uri()->scheme() == "https" )
		{
			dict = [ NSDictionary dictionaryWithObjectsAndKeys:
									[ NSString stringWithUTF8String:m_request->proxy()->host().c_str() ], kCFStreamPropertyHTTPSProxyHost,
									[ NSNumber numberWithInt:m_request->proxy()->port() ], kCFStreamPropertyHTTPSProxyPort,
									nil ];
		}
		else
		{
			dict = [ NSDictionary dictionaryWithObjectsAndKeys:
									[ NSString stringWithUTF8String:m_request->proxy()->host().c_str() ], kCFStreamPropertyHTTPProxyHost,
									[ NSNumber numberWithInt:m_request->proxy()->port() ], kCFStreamPropertyHTTPProxyPort,
									nil ];
		}
								
		CFReadStreamSetProperty( m_stream, kCFStreamPropertyHTTPProxy, ( CFDictionaryRef ) dict );
	}
		
	CFReadStreamOpen( m_stream );
   
	if ( !CFReadStreamSetClient( m_stream, kCFStreamEventHasBytesAvailable | kCFStreamEventErrorOccurred | kCFStreamEventEndEncountered, event_callback, &context ) )
	{
		nklog( log::error, "CFReadStreamSetClient() failed" );
		reply( -1 );
		goto exit;
	}
	
	CFReadStreamScheduleWithRunLoop( m_stream, CFRunLoopGetCurrent(),kCFRunLoopCommonModes );
	
	m_response->body().clear();
	
	m_request->new_try();
	
exit:

	if ( handle )
	{
		CFRelease( handle );
	}
}


void
client_mac::close_stream()
{
	if ( m_stream )
	{
		CFReadStreamSetClient( m_stream, kCFStreamEventNone, NULL, NULL );
		CFReadStreamUnscheduleFromRunLoop( m_stream, CFRunLoopGetCurrent(), kCFRunLoopCommonModes );
		CFReadStreamClose( m_stream );
		CFRelease( m_stream );
		m_stream = NULL;
	}
}


void
client_mac::reply( int32_t error )
{
	m_response_func( error, m_response );
	
	close_stream();
	
	release();
}