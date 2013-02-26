#ifndef _netkit_sink_h
#define _netkit_sink_h

#include <NetKit/NKObject.h>

namespace netkit {

class source;
typedef smart_ptr< source > source_ptr;

class sink : public object
{
public:

	typedef smart_ptr< sink > ptr;
	
	sink( const source_ptr &source );
	
	virtual ~sink();
	
	virtual ssize_t
	process() = 0;
	
	ssize_t
	recv( std::uint8_t *buf, size_t len );
	
	ssize_t
	send( const std::uint8_t *buf, size_t len );
	
private:

	source_ptr m_source;
};

}

#endif