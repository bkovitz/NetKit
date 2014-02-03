#include <NetKit/NKNetworkInterface.h>
#include <NetKit/NKUnicode.h>
#include <NetKit/NKJSON.h>
#include <NetKit/NKMacros.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <errno.h>
#include <stdio.h>

using namespace netkit;

nic::array
nic::instances()
{
	nic::array		nics;
	struct ifaddrs	*interfaces = nullptr;
	struct ifaddrs	*interface = nullptr;
	void			*temp_addr = NULL;
	int				rc = 0;
  
	rc = getifaddrs( &interfaces );
	
	if ( rc == 0 )
	{
		for ( interface = interfaces; interface != nullptr; interface = interface->ifa_next )
		{
			if ( interface->ifa_addr->sa_family == AF_INET )
			{
				temp_addr = &((struct sockaddr_in *) interface->ifa_addr)->sin_addr;
			}
			else
			{
				temp_addr = &((struct sockaddr_in6 *) interface->ifa_addr)->sin6_addr;
			}
      
			//printf("LineDescription :  %s \n", tempIfAddr->ifa_name);
		}
	
		if ( interfaces )
		{
			freeifaddrs( interfaces );
		}
	}
	else
	{
		nklog( log::error, "getifaddrs() failed: %d", errno );
	}
	
	return nics;


#if 0



	nic::array					nics;
	std::vector< std::uint8_t > bytes;
	auto						flags = GAA_FLAG_INCLUDE_PREFIX | GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER;
    auto						i = 0;

	for ( ;; )
    {
		ULONG size = 0;

		auto err = GetAdaptersAddresses( AF_UNSPEC, flags, NULL, NULL, &size );
		netkit_require( err == ERROR_BUFFER_OVERFLOW, exit );
		netkit_require( size >= sizeof( IP_ADAPTER_ADDRESSES ), exit );

		bytes.resize( size );

		err = GetAdaptersAddresses( AF_UNSPEC, flags, NULL, ( PIP_ADAPTER_ADDRESSES ) &bytes[ 0 ], &size );

		if ( err == ERROR_SUCCESS )
		{
			break;
		}

        ++i;

		netkit_require( i < 100, exit );
	}

	for ( auto addr = ( PIP_ADAPTER_ADDRESSES ) &bytes[ 0 ]; addr; addr = addr->Next )
    {
		auto						nic		= new netkit::nic;
		PIP_ADAPTER_UNICAST_ADDRESS unicast = nullptr;

		nic->m_name			= addr->AdapterName;
		nic->m_display_name	= narrow( addr->FriendlyName );

		if ( addr->OperStatus == IfOperStatusUp )
		{
			nic->m_flags |= nic::flags::up;
		}

		if ( addr->IfType == IF_TYPE_SOFTWARE_LOOPBACK )
		{
			nic->m_flags |= nic::flags::loopback;
		}

		if ( !( addr->Flags & IP_ADAPTER_NO_MULTICAST ) )
		{
			nic->m_flags |= nic::flags::multicast;
		}

		for ( unicast = addr->FirstUnicastAddress; unicast ; unicast = unicast->Next )
		{
			if ( ( unicast->Address.lpSockaddr->sa_family == AF_INET ) ||
			     ( unicast->Address.lpSockaddr->sa_family == AF_INET6 ) )
			{
				sockaddr_storage storage;

				if ( addr->DnsSuffix )
				{
					nic->m_dns_suffix = narrow( addr->DnsSuffix );
				}

				memset( &storage, 0, sizeof( storage ) );
				memcpy( &storage, unicast->Address.lpSockaddr, unicast->Address.iSockaddrLength );

				nic->m_addresses.push_back( dynamic_cast< ip::address* >( ip::address::from_sockaddr( storage ).get() ) ); 

			}
		}

		nics.push_back( nic );
	}

exit:

	return nics;
#endif
}