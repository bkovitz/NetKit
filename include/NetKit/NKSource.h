#ifndef _netkit_source_h
#define _netkit_source_h

#include <NetKit/NKSink.h>
#include <initializer_list>
#include <list>

namespace netkit {

class source : public object
{
public:

	typedef smart_ptr< source > ptr;
	
	source();
	
	virtual ~source();

	sink::ptr
	sink() const;
	
	void
	bind( sink::ptr sink );
	
	virtual ssize_t
	peek( std::uint8_t *buf, size_t len ) = 0;
	
	virtual ssize_t
	read( std::uint8_t *buf, size_t len ) = 0;
	
	virtual ssize_t
	send( const std::uint8_t *buf, size_t len ) = 0;
	
protected:

	sink::ptr m_sink;
};

}

#endif
