#include "NKHTTP_Mac.h"
#include <NetKit/NKLog.h>

using namespace netkit::http;

#if defined( __APPLE__ )
#	pragma mark client_mac implementation
#endif

client_mac::client_mac( const request::ptr &request, auth_f auth_func, response_f response_func )
:
	client( request, auth_func, response_func ),
	m_stream( NULL )
{
	m_response = new http::response;
}


client_mac::~client_mac()
{
	close_stream();
}


void
client::send( const request::ptr &request, response_f response_func )
{
	client_mac *self = new client_mac( request, [=]( request::ptr &request, uint32_t status )
	{
		return false;
	}, response_func );

	self->send_request( NULL );
}



void
client::send( const request::ptr &request, auth_f auth_func, response_f response_func )
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
	CFHTTPMessageRef		handle	= CFHTTPMessageCreateRequest( kCFAllocatorDefault, ( CFStringRef ) [ NSString stringWithUTF8String:http::method::as_string( m_request->method() ).c_str() ], ( CFURLRef ) [ NSURL URLWithString:[ NSString stringWithUTF8String:m_request->uri()->recompose().c_str() ] ], kCFHTTPVersion1_1 );
	
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