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

#include <NetKit/NetKit.h>
#include <Foundation/Foundation.h>
#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SystemConfiguration.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/network/IOEthernetInterface.h>
#include <IOKit/network/IONetworkInterface.h>
#include <IOKit/network/IOEthernetController.h>
#include <sys/param.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <sys/stat.h>

#include <net/if.h>
#include <net/if_dl.h>
#include <netinet/in.h>
#include <net/if_types.h>
#include <net/route.h>
#include <netinet/if_ether.h>

#include <arpa/inet.h>

#include <err.h>
#include <errno.h>
#include <netdb.h>

#include <paths.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>


static kern_return_t
find_ethernet_interfaces( io_iterator_t *matchingServices );


static kern_return_t
get_mac_address( io_iterator_t intfIterator, UInt8 *MACAddress, UInt8 bufferSize );


static std::string
serial_number();


using namespace netkit;


void
netkit::initialize()
{
	static bool					first = true;
	
	if ( first )
	{
		netkit::component::m_instances					= new netkit::component::list;
		
		netkit::http::server::m_connections				= new netkit::http::connection::list;
		
		netkit::json::server::m_connections				= new netkit::json::connection::list;
		netkit::json::server::m_notification_handlers	= new netkit::json::server::notification_handlers;
		netkit::json::server::m_request_handlers		= new netkit::json::server::request_handlers;
		
		first = false;
	}
}


std::string
platform::machine_name()
{
	static std::string name;
	
	if ( name.size() == 0 )
	{
		CFStringEncoding	encoding	= kCFStringEncodingUTF8;
		CFStringRef			cfs			= SCDynamicStoreCopyComputerName( NULL, &encoding );
    
		if ( cfs )
		{
			char buf[ 256 ];

			CFStringGetCString( cfs, buf, sizeof( buf ), kCFStringEncodingUTF8 );
			name = buf;
			CFRelease( cfs );
		}
		else
		{
			name = "localhost";
		}
	}
	
	return name;
}


std::string
platform::machine_description()
{
	static std::string description;
	
	if ( description.size() == 0 )
	{
		NSString		*systemVersionFile	= @"/System/Library/CoreServices/SystemVersion.plist";
		NSData			*data				= [ NSData dataWithContentsOfFile:systemVersionFile ];
		NSDictionary	*dict				= [ NSPropertyListSerialization propertyListFromData:data mutabilityOption:NSPropertyListImmutable format:NULL errorDescription:nil ];
		NSString		*temp				= [ NSString stringWithFormat:@"%@ %@ (%@)", [ dict objectForKey:@"ProductName" ], [ dict objectForKey:@"ProductVersion" ], [ dict objectForKey:@"ProductBuildVersion" ] ];
	
		description = [ temp UTF8String ];
	}
	
	return description;
}


std::string
platform::machine_id()
{
	std::string id;
	
	if ( id.size() == 0 )
	{
		kern_return_t	kernResult = KERN_SUCCESS;
		io_iterator_t	intfIterator;
		UInt8			MACAddress[kIOEthernetAddressSize];
 
		kernResult = find_ethernet_interfaces(&intfIterator);
    
		if ( KERN_SUCCESS != kernResult)
		{
			printf("find_ethernet_interfaces returned 0x%08x\n", kernResult);
		}
		else
		{
			kernResult = get_mac_address(intfIterator, MACAddress, sizeof(MACAddress));
        
			if ( KERN_SUCCESS != kernResult )
			{
				id = serial_number();
			}
			else
			{
				NSString *serial = [ NSString stringWithFormat:@"%02x:%02x:%02x:%02x:%02x:%02x", MACAddress[ 0 ], MACAddress[ 1 ], MACAddress[ 2 ], MACAddress[ 3 ], MACAddress[ 4 ], MACAddress[ 5 ] ];
				id = [ serial UTF8String ];
			}
		}
    
		( void ) IOObjectRelease(intfIterator);   // Release the iterator.
	}

	return id;
}


std::string
platform::username()
{
	return [ NSUserName() UTF8String ];
}


std::string
platform::make_filesystem_safe( const std::string &name )
{
	std::string safe;

    for ( auto it = name.begin(); it != name.end(); it++ )
    {
        if ( ( *it == ':' ) || ( *it == '\n' ) )
        {
            safe.push_back( '_' );
        }
        else
        {
            safe.push_back( *it );
        }
    }

    return safe;
}


bool
platform::create_folder( const std::string &folder )
{
	//return SHCreateDirectoryEx( NULL, folder, NULL ) == ERROR_SUCCESS ? true : false;
	return false;
}


bool
platform::copy_file( const std::string &from, const std::string &to )
{
	return false;
}


bool
platform::file_exists( const std::string &name )
{
	struct stat buf;
	return ::stat( name.c_str(), &buf ) == 0 ? true : false;
}


// Returns an iterator containing the primary (built-in) Ethernet interface. The caller is responsible for
// releasing the iterator after the caller is done with it.
static kern_return_t
find_ethernet_interfaces(io_iterator_t *matchingServices)
{
    kern_return_t           kernResult; 
    CFMutableDictionaryRef  matchingDict;
    CFMutableDictionaryRef  propertyMatchDict;
    
    // Ethernet interfaces are instances of class kIOEthernetInterfaceClass. 
    // IOServiceMatching is a convenience function to create a dictionary with the key kIOProviderClassKey and 
    // the specified value.
    matchingDict = IOServiceMatching(kIOEthernetInterfaceClass);
 
    // Note that another option here would be:
    // matchingDict = IOBSDMatching("en0");
    // but en0: isn't necessarily the primary interface, especially on systems with multiple Ethernet ports.
        
    if (NULL == matchingDict)
	{
        printf("IOServiceMatching returned a NULL dictionary.\n");
    }
    else
	{
        // Each IONetworkInterface object has a Boolean property with the key kIOPrimaryInterface. Only the
        // primary (built-in) interface has this property set to TRUE.
        
        // IOServiceGetMatchingServices uses the default matching criteria defined by IOService. This considers
        // only the following properties plus any family-specific matching in this order of precedence 
        // (see IOService::passiveMatch):
        //
        // kIOProviderClassKey (IOServiceMatching)
        // kIONameMatchKey (IOServiceNameMatching)
        // kIOPropertyMatchKey
        // kIOPathMatchKey
        // kIOMatchedServiceCountKey
        // family-specific matching
        // kIOBSDNameKey (IOBSDNameMatching)
        // kIOLocationMatchKey
        
        // The IONetworkingFamily does not define any family-specific matching. This means that in            
        // order to have IOServiceGetMatchingServices consider the kIOPrimaryInterface property, we must
        // add that property to a separate dictionary and then add that to our matching dictionary
        // specifying kIOPropertyMatchKey.
            
        propertyMatchDict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                                      &kCFTypeDictionaryKeyCallBacks,
                                                      &kCFTypeDictionaryValueCallBacks);
    
        if (NULL == propertyMatchDict) {
            printf("CFDictionaryCreateMutable returned a NULL dictionary.\n");
        }
        else {
            // Set the value in the dictionary of the property with the given key, or add the key 
            // to the dictionary if it doesn't exist. This call retains the value object passed in.
            CFDictionarySetValue(propertyMatchDict, CFSTR(kIOPrimaryInterface), kCFBooleanTrue); 
            
            // Now add the dictionary containing the matching value for kIOPrimaryInterface to our main
            // matching dictionary. This call will retain propertyMatchDict, so we can release our reference 
            // on propertyMatchDict after adding it to matchingDict.
            CFDictionarySetValue(matchingDict, CFSTR(kIOPropertyMatchKey), propertyMatchDict);
            CFRelease(propertyMatchDict);
        }
    }
    
    // IOServiceGetMatchingServices retains the returned iterator, so release the iterator when we're done with it.
    // IOServiceGetMatchingServices also consumes a reference on the matching dictionary so we don't need to release
    // the dictionary explicitly.

    kernResult = IOServiceGetMatchingServices(kIOMasterPortDefault, matchingDict, matchingServices);    

    if (KERN_SUCCESS != kernResult)
	{
        printf("IOServiceGetMatchingServices returned 0x%08x\n", kernResult);
    }
        
    return kernResult;
}
    

// Given an iterator across a set of Ethernet interfaces, return the MAC address of the last one.
// If no interfaces are found the MAC address is set to an empty string.
// In this sample the iterator should contain just the primary interface.
static kern_return_t
get_mac_address(io_iterator_t intfIterator, UInt8 *MACAddress, UInt8 bufferSize)
{
    io_object_t     intfService;
    io_object_t     controllerService;
    kern_return_t   kernResult = KERN_FAILURE;
    
    // Make sure the caller provided enough buffer space. Protect against buffer overflow problems.
    if (bufferSize < kIOEthernetAddressSize) {
        return kernResult;
    }
    
    // Initialize the returned address
    bzero(MACAddress, bufferSize);
    
    // IOIteratorNext retains the returned object, so release it when we're done with it.
    while ((intfService = IOIteratorNext(intfIterator)))
    {
        CFTypeRef   MACAddressAsCFData;        
 
        // IONetworkControllers can't be found directly by the IOServiceGetMatchingServices call, 
        // since they are hardware nubs and do not participate in driver matching. In other words,
        // registerService() is never called on them. So we've found the IONetworkInterface and will 
        // get its parent controller by asking for it specifically.
        
        // IORegistryEntryGetParentEntry retains the returned object, so release it when we're done with it.
        kernResult = IORegistryEntryGetParentEntry(intfService,
                                                   kIOServicePlane,
                                                   &controllerService);
        
        if (KERN_SUCCESS != kernResult) {
            printf("IORegistryEntryGetParentEntry returned 0x%08x\n", kernResult);
        }
        else {
            // Retrieve the MAC address property from the I/O Registry in the form of a CFData
            MACAddressAsCFData = IORegistryEntryCreateCFProperty(controllerService,
                                                                 CFSTR(kIOMACAddress),
                                                                 kCFAllocatorDefault,
                                                                 0);
            if (MACAddressAsCFData) {
                CFShow(MACAddressAsCFData); // for display purposes only; output goes to stderr
                
                // Get the raw bytes of the MAC address from the CFData
                CFDataGetBytes(( CFDataRef ) MACAddressAsCFData, CFRangeMake(0, kIOEthernetAddressSize), MACAddress);
                CFRelease(MACAddressAsCFData);
            }
                
            // Done with the parent Ethernet controller object so we release it.
            (void) IOObjectRelease(controllerService);
        }
        
        // Done with the Ethernet interface object so we release it.
        (void) IOObjectRelease(intfService);
    }
        
    return kernResult;
}
 

static std::string
serial_number()
{
	std::string serialNumber;

	io_service_t platformExpert = IOServiceGetMatchingService(kIOMasterPortDefault, IOServiceMatching("IOPlatformExpertDevice" ) );

	if ( platformExpert )
	{
		CFTypeRef serialNumberAsCFString = IORegistryEntryCreateCFProperty(platformExpert, CFSTR(kIOPlatformSerialNumberKey), kCFAllocatorDefault, 0);

		if ( serialNumberAsCFString)
		{
			serialNumber = [ ( NSString* ) serialNumberAsCFString UTF8String ];
		}

		IOObjectRelease(platformExpert);
		
    }

	return serialNumber;
}


uuid::ref
uuid::create()
{
	CFUUIDRef       uuidRef     =   CFUUIDCreate( NULL );
	CFUUIDBytes     uuidBytes   =   CFUUIDGetUUIDBytes( uuidRef );
	std::uint8_t	data[ 16 ];
	
	data[ 0 ] = uuidBytes.byte0;
	data[ 1 ] = uuidBytes.byte1;
	data[ 2 ] = uuidBytes.byte2;
	data[ 3 ] = uuidBytes.byte3;
	data[ 4 ] = uuidBytes.byte4;
	data[ 5 ] = uuidBytes.byte5;
	data[ 6 ] = uuidBytes.byte6;
	data[ 7 ] = uuidBytes.byte7;
	data[ 8 ] = uuidBytes.byte8;
	data[ 9 ] = uuidBytes.byte9;
	data[ 10 ] = uuidBytes.byte10;
	data[ 11 ] = uuidBytes.byte11;
	data[ 12 ] = uuidBytes.byte12;
	data[ 13 ] = uuidBytes.byte13;
	data[ 14 ] = uuidBytes.byte14;
	data[ 15 ] = uuidBytes.byte15;

	CFRelease( uuidRef );

	return new uuid( data );
}