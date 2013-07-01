include_directories (${NetKit_SOURCE_DIR}/include)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fpermissive")

add_executable (address test_address.cpp catch.hpp)

target_link_libraries (address NetKit)

#main.cpp
#test_address.cpp
#test_http.cpp
#test_json.cpp
#test_socket.cpp
#test_ssl.cpp
#test_uri.cpp
#test_uuid.cpp
#test_ws.cpp
