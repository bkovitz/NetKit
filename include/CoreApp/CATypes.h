#ifndef _CoreApp_types_h
#define _CoreApp_types_h

#include <cstdint>
#include <vector>

#if defined( WIN32 )

#	include <winsock2.h>
#	include <ws2tcpip.h>
#	include <windows.h>

#else

#	define TEXT( X ) X

#endif

namespace CoreApp {

using std::int8_t;
using std::uint8_t;
using std::int16_t;
using std::uint16_t;
using std::int32_t;
using std::uint32_t;
using std::int64_t;
using std::uint64_t;

typedef std::vector< uint8_t > blob;
typedef uint8_t *buf_t;

}

#endif
