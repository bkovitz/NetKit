#include <NetKit/NKPlatform.h>
#include <NetKit/NKUnicode.h>
#include <NetKit/NKComponent.h>
#include <NetKit/NKHTTP.h>
#include <NetKit/NKJSON.h>
#include <NetKit/NKLog.h>
#include <winsock2.h>
#include <iphlpapi.h>
#include <windows.h>
#include <tchar.h>
#include <sddl.h>
#include <shlobj.h>
#include <shellapi.h>
#include <strsafe.h>
#include "CRC32.h"

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "User32.lib")

using namespace netkit;

void
netkit::initialize()
{
    static bool                 first = true;

    if ( first )
    {
		WSADATA WSAData;

		WSAStartup( MAKEWORD( 2, 2 ), &WSAData );

        netkit::component::m_instances                  = new netkit::component::list;

        netkit::http::connection::m_instances           = new netkit::http::connection::list;

        netkit::json::connection::m_instances           = new netkit::json::connection::list;
        netkit::json::server::m_notification_handlers   = new netkit::json::server::notification_handlers;
        netkit::json::server::m_request_handlers        = new netkit::json::server::request_handlers;

        first = false;
    }
}


std::string
platform::machine_name()
{
	static std::string name;

	if ( name.length() == 0 )
	{
		TCHAR	buf[ 256 ];
		DWORD	bufLen = sizeof( buf );
		
		if ( !GetComputerName( buf, &bufLen ) )
		{
			_tcscpy_s( buf, sizeof( buf ) / sizeof( TCHAR ), TEXT( "localhost" ) );
		}

		name = narrow( buf );
	}
	
	return name;
}


std::string
platform::machine_description()
{
	static std::string description;

	if ( description.length() == 0 )
	{
		typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
		typedef BOOL (WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);

		TCHAR			pszOS[ 1024 ];
		OSVERSIONINFOEX osvi;
		SYSTEM_INFO		si;
		PGNSI			pGNSI;
		PGPI			pGPI;
		DWORD			dwType;

		ZeroMemory(&si, sizeof(SYSTEM_INFO));
		ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));

		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

		if ( !GetVersionEx( ( OSVERSIONINFO* ) &osvi) )
		{
			description = "Windows";
			goto exit;
		}

		// Call GetNativeSystemInfo if supported or GetSystemInfo otherwise.

		pGNSI = (PGNSI) GetProcAddress( GetModuleHandle( TEXT( "kernel32.dll" ) ), "GetNativeSystemInfo" );

		if( pGNSI != NULL )
		{
			pGNSI( &si );
		}
		else
		{
			GetSystemInfo( &si );
		}

		if ( ( VER_PLATFORM_WIN32_NT == osvi.dwPlatformId ) && ( osvi.dwMajorVersion > 4 ) )
		{
			StringCchCopy( pszOS, sizeof( pszOS ), TEXT( "Microsoft " ) );

			if ( osvi.dwMajorVersion == 6 )
      		{
				if ( osvi.dwMinorVersion == 0 )
				{
					if ( osvi.wProductType == VER_NT_WORKSTATION )
					{
                		StringCchCat( pszOS, sizeof( pszOS ), TEXT("Windows Vista " ) );
					}
					else
					{
						StringCchCat( pszOS, sizeof( pszOS ), TEXT( "Windows Server 2008 " ) );
					}
         		}
         		else if ( osvi.dwMinorVersion == 1 )
				{
					if ( osvi.wProductType == VER_NT_WORKSTATION )
					{
                		StringCchCat( pszOS, sizeof( pszOS ), TEXT("Windows 7 " ) );
					}
					else
					{
						StringCchCat( pszOS, sizeof( pszOS ), TEXT( "Windows Server 2008 R2 " ) );
         			}
				}
         
				pGPI = (PGPI) GetProcAddress( GetModuleHandle(TEXT("kernel32.dll")), "GetProductInfo" );

				if ( pGPI )
				{
					pGPI( osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.wServicePackMajor, osvi.wServicePackMinor, &dwType);

					switch( dwType )
					{
            			case PRODUCT_ULTIMATE:
						{
               				StringCchCat( pszOS, sizeof( pszOS ), TEXT("Ultimate Edition" ) );
						}
               			break;

						case PRODUCT_PROFESSIONAL:
						{
               				StringCchCat(pszOS, sizeof( pszOS ), TEXT("Professional" ));
						}
               			break;

						case PRODUCT_HOME_PREMIUM:
						{
               				StringCchCat(pszOS, sizeof( pszOS ), TEXT("Home Premium Edition" ));
						}
               			break;

						case PRODUCT_HOME_BASIC:
						{
               				StringCchCat(pszOS, sizeof( pszOS ), TEXT("Home Basic Edition" ));
						}
               			break;

            			case PRODUCT_ENTERPRISE:
						{
               				StringCchCat(pszOS, sizeof( pszOS ), TEXT("Enterprise Edition" ));
						}
               			break;

            			case PRODUCT_BUSINESS:
						{
               				StringCchCat(pszOS, sizeof( pszOS ), TEXT("Business Edition" ));
						}
               			break;

            			case PRODUCT_STARTER:
						{
             				StringCchCat(pszOS, sizeof( pszOS ), TEXT("Starter Edition" ));
						}
               			break;

            			case PRODUCT_CLUSTER_SERVER:
						{
               				StringCchCat(pszOS, sizeof( pszOS ), TEXT("Cluster Server Edition" ));
						}
               			break;

						case PRODUCT_DATACENTER_SERVER:
						{
              				StringCchCat(pszOS, sizeof( pszOS ), TEXT("Datacenter Edition" ));
						}
               			break;

			            case PRODUCT_DATACENTER_SERVER_CORE:
						{
               				StringCchCat(pszOS, sizeof( pszOS ), TEXT("Datacenter Edition (core installation)" ));
						}
               			break;

            			case PRODUCT_ENTERPRISE_SERVER:
						{
               				StringCchCat(pszOS, sizeof( pszOS ), TEXT("Enterprise Edition" ));
						}
               			break;

			            case PRODUCT_ENTERPRISE_SERVER_CORE:
						{
               				StringCchCat(pszOS, sizeof( pszOS ), TEXT("Enterprise Edition (core installation)" ));
						}
               			break;

            			case PRODUCT_ENTERPRISE_SERVER_IA64:
						{
               				StringCchCat(pszOS, sizeof( pszOS ), TEXT("Enterprise Edition for Itanium-based Systems" ));
						}
               			break;

			            case PRODUCT_SMALLBUSINESS_SERVER:
						{
               				StringCchCat(pszOS, sizeof( pszOS ), TEXT("Small Business Server" ));
						}
               			break;

			            case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
						{
               				StringCchCat(pszOS, sizeof( pszOS ), TEXT("Small Business Server Premium Edition" ));
						}
               			break;

            			case PRODUCT_STANDARD_SERVER:
						{
               				StringCchCat(pszOS, sizeof( pszOS ), TEXT("Standard Edition" ));
						}
               			break;

			            case PRODUCT_STANDARD_SERVER_CORE:
						{
               				StringCchCat(pszOS, sizeof( pszOS ), TEXT("Standard Edition (core installation)" ));
						}
               			break;

			            case PRODUCT_WEB_SERVER:
						{
               				StringCchCat(pszOS, sizeof( pszOS ), TEXT("Web Server Edition" ));
						}
               			break;
         			}
      			}
			}
			else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 )
      		{
				if( GetSystemMetrics(SM_SERVERR2) )
				{
            		StringCchCat(pszOS, sizeof( pszOS ), TEXT( "Windows Server 2003 R2, "));
				}
				else if ( osvi.wSuiteMask & VER_SUITE_STORAGE_SERVER )
				{
            		StringCchCat(pszOS, sizeof( pszOS ), TEXT( "Windows Storage Server 2003"));
				}
         		else if ( osvi.wSuiteMask & VER_SUITE_WH_SERVER )
				{
            		StringCchCat(pszOS, sizeof( pszOS ), TEXT( "Windows Home Server"));
				}
         		else if( osvi.wProductType == VER_NT_WORKSTATION && si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64)
         		{
					StringCchCat(pszOS, sizeof( pszOS ), TEXT( "Windows XP Professional x64 Edition"));
         		}
         		else
				{
					StringCchCat(pszOS, sizeof( pszOS ), TEXT("Windows Server 2003, "));
				}

         		if ( osvi.wProductType != VER_NT_WORKSTATION )
         		{
					if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_IA64 )
            		{
						if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
						{
                   			StringCchCat(pszOS, sizeof( pszOS ), TEXT( "Datacenter Edition for Itanium-based Systems" ));
						}
						else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
						{
                   			StringCchCat(pszOS, sizeof( pszOS ), TEXT( "Enterprise Edition for Itanium-based Systems" ));
						}
            		}
            		else if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64 )
            		{
                		if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
						{
                   			StringCchCat(pszOS, sizeof( pszOS ), TEXT( "Datacenter x64 Edition" ));
						}
                		else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
						{
                   			StringCchCat(pszOS, sizeof( pszOS ), TEXT( "Enterprise x64 Edition" ));
						}
						else
						{
							StringCchCat(pszOS, sizeof( pszOS ), TEXT( "Standard x64 Edition" ));
						}
            		}
            		else
            		{
                		if ( osvi.wSuiteMask & VER_SUITE_COMPUTE_SERVER )
						{
                   			StringCchCat(pszOS, sizeof( pszOS ), TEXT( "Compute Cluster Edition" ));
						}
                		else if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
						{
                   			StringCchCat(pszOS, sizeof( pszOS ), TEXT( "Datacenter Edition" ));
						}
                		else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
						{
                   			StringCchCat(pszOS, sizeof( pszOS ), TEXT( "Enterprise Edition" ));
						}
                		else if ( osvi.wSuiteMask & VER_SUITE_BLADE )
						{
                   			StringCchCat(pszOS, sizeof( pszOS ), TEXT( "Web Edition" ));
						}
                		else
						{
							StringCchCat(pszOS, sizeof( pszOS ), TEXT( "Standard Edition" ));
						}
            		}
         		}
			}
      		else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 )
      		{
         		StringCchCat(pszOS, sizeof( pszOS ), TEXT("Windows XP "));

				if( osvi.wSuiteMask & VER_SUITE_PERSONAL )
				{
            		StringCchCat(pszOS, sizeof( pszOS ), TEXT( "Home Edition" ));
				}
   				else
				{
					StringCchCat(pszOS, sizeof( pszOS ), TEXT( "Professional" ));
      			}
			}
			else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
      		{
         		StringCchCat(pszOS, sizeof( pszOS ), TEXT("Windows 2000 "));

         		if ( osvi.wProductType == VER_NT_WORKSTATION )
         		{
            		StringCchCat(pszOS, sizeof( pszOS ), TEXT( "Professional" ));
         		}
         		else 
         		{
            		if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
					{
               			StringCchCat(pszOS, sizeof( pszOS ), TEXT( "Datacenter Server" ));
					}
            		else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
					{
               			StringCchCat(pszOS, sizeof( pszOS ), TEXT( "Advanced Server" ));
					}
            		else
					{
						StringCchCat(pszOS, sizeof( pszOS ), TEXT( "Server" ));
					}
         		}
      		}

			// Include service pack (if any) and build number.

			if ( _tcslen(osvi.szCSDVersion) > 0 )
      		{
          		StringCchCat(pszOS, sizeof( pszOS ), TEXT(" ") );
          		StringCchCat(pszOS, sizeof( pszOS ), osvi.szCSDVersion);
      		}

			TCHAR buf[80];

			StringCchPrintf( buf, 80, TEXT(" (build %d)"), osvi.dwBuildNumber);
			StringCchCat(pszOS, sizeof( pszOS ), buf);

			if ( osvi.dwMajorVersion >= 6 )
			{
				if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64 )
				{
            		StringCchCat(pszOS, sizeof( pszOS ), TEXT( ", 64-bit" ));
				}
				else if (si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_INTEL )
				{
		            StringCchCat(pszOS, sizeof( pszOS ), TEXT(", 32-bit"));
				}
			}
      	}

		description = narrow( pszOS );
	}

exit:
      
	return description;
}


std::string
platform::machine_id()
{
	static std::string id;

	if ( id.length() == 0 )
	{
		IP_ADAPTER_INFO adapterInfo[16];	// Allocate information for up to 16 NICs
		DWORD dwBufLen = sizeof( adapterInfo );  // Save memory size of buffer
		DWORD dwStatus = GetAdaptersInfo( adapterInfo, &dwBufLen );
		PIP_ADAPTER_INFO pAdapterInfo;

		if ( dwStatus != ERROR_SUCCESS )
		{
			nklog( log::error, "GetAdaptersInfo() failed: %d", GetLastError() );
			id = uuid();
			goto exit;
		}
 
		pAdapterInfo = adapterInfo; // Contains pointer to current adapter info
		
		while ( pAdapterInfo )
		{
			if ( pAdapterInfo->AddressLength == 6 )
			{
				char buf[ 1024 ];

				_snprintf_s( buf, sizeof( buf ), sizeof( buf ), "%02X:%02X:%02X:%02X:%02X:%02X", 
					 pAdapterInfo->Address[0],
					 pAdapterInfo->Address[1],
					 pAdapterInfo->Address[2],
					 pAdapterInfo->Address[3],
					 pAdapterInfo->Address[4],
					 pAdapterInfo->Address[5] );

				id = buf;

				break;
			}

			pAdapterInfo = pAdapterInfo->Next;
		}
	}

exit:

	return id;
}


std::string
platform::uuid()
{
	char	buf[ 1024 ];
	GUID	guid;

	if ( CoCreateGuid( &guid ) == S_OK )
	{
		sprintf_s( buf, sizeof( buf ), "%.08x-%.04x-%.04x-%.02x%.02x-%.02x%.02x%.02x%.02x%.02x%.02x", guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
	}
	else
	{
		// fall back in case of failure - build a custom GUID
		LARGE_INTEGER cpuTime;

		QueryPerformanceCounter(&cpuTime);
 
		long guidValue = CRC32( ( const unsigned char* ) &cpuTime, sizeof( cpuTime ) );
 
		sprintf_s( buf, sizeof( buf ), "%08x-%04x-%04x-%04x-%08x%04x", guidValue, ( guidValue >> 16 ), ( guidValue & 0xFFFF ) ^ ( guidValue >> 16 ), ( long )( cpuTime.HighPart & 0xFFFF ), ( long )( cpuTime.QuadPart / 0xF1130495 ), GetTickCount() & 0xFFFF );
	}

	return buf;
}


bool
platform::create_folder( const std::string &folder )
{
	return SHCreateDirectoryEx( NULL, widen( folder ).c_str(), NULL ) == ERROR_SUCCESS ? true : false;
}

