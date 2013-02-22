#ifndef _netkit_ip_address_h
#define _netkit_ip_address_h

#include <NetKit/NKObject.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <netinet/in.h>
#include <deque>


namespace netkit {
namespace ip {

class address : public object
{
public:

	typedef std::function< void ( int status, std::deque< smart_ptr< address > > &addrs ) > resolve_reply;

	typedef smart_ptr< address > ptr;
	typedef std::deque< ptr > list;
	
public:

	address( uint32_t addr, uint16_t );
	
	address( struct in_addr addr, uint16_t port );
	
	address( struct in6_addr addr, uint16_t port );
	
	address( sockaddr_storage sockaddr );

	virtual ~address();
	
	static void
	resolve( std::string host, uint16_t port, resolve_reply reply );
	
	inline sockaddr_storage
	sockaddr() const
	{
		return m_native;
	}
	
	std::string
	host() const;
	
	uint16_t
	port() const;
	
	void
	set_port( uint16_t port )
	{
		if ( m_native.ss_family == AF_INET )
		{
			( ( struct sockaddr_in* ) &m_native )->sin_port = htons( port );
		}
		else if ( m_native.ss_family == AF_INET6 )
		{
			( ( struct sockaddr_in6* ) &m_native )->sin6_port = htons( port );
		}
	}
	
	inline bool
	operator==( const address::ptr &that ) const
	{
		return operator==( that.operator->() );
	}
	
	inline bool
	operator==( const address * that ) const
	{
		if ( !that )
		{
			return false;
		}
		else
		{
			return memcmp( &m_native, &that->m_native, sizeof( m_native ) ) == 0;
		}
	}

protected:

	struct sockaddr_storage m_native;
};

}
}

#endif
