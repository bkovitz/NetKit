set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
include_directories (${NetKit_SOURCE_DIR}/include)

set(NETKIT_SOURCE 
		NKAddress.cpp
		NKBase64.cpp
		NKComponent.cpp
		NKConnection.cpp
		NKDatabase_SQLite.cpp
		NKDatabase_SQLite.h
		NKEndpoint.cpp
		NKError.cpp
		NKHTTP.cpp
		NKJSON.cpp
		NKLog.cpp
		NKMIME.cpp
		NKNetworkInterface.cpp
		NKOAuth.cpp
		NKObject.cpp
		NKProxy.cpp
		NKSHA1.cpp
		NKSink.cpp
		NKSocket.cpp
		NKSource.cpp
		NKTLS.cpp
		NKURI.cpp
		NKUUID.cpp
		NKUnicode.cpp
		NKWebSocket.cpp
		NKXMLAttribute.cpp
		NKXMLDocument.cpp
		NKXMLElement.cpp
		NKXMLNode.cpp
		NKXMPP.cpp
)

add_library (NetKit ${NETKIT_SOURCE})
