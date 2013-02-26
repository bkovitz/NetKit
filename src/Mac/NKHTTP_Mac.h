#ifndef _netkit_http_mac_h
#define _netkit_http_mac_h

#include <NetKit/NKHTTP.h>
#include <CoreFoundation/CoreFoundation.h>
#include <SecurityFoundation/SFAuthorization.h>

namespace netkit {

namespace http {

class client_mac : public client
{
public:

	client_mac( const request::ptr &request, auth_f auth_func, response_f response_func );
	
	virtual ~client_mac();
	
	void
	send_request( CFHTTPAuthenticationRef auth );

protected:

	static void
	event_callback( CFReadStreamRef stream, CFStreamEventType eventType, void *clientcallBackInfo );
	
	void
	reply( int32_t error );

	void
	close_stream();

	CFReadStreamRef m_stream;
};

}

}

#endif
