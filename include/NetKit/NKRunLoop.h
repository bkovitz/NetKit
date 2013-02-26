#ifndef _netkit_runloop_h
#define _netkit_runloop_h

#include <NetKit/NKObject.h>
#include <NetKit/NKSocket.h>
#include <functional>
#include <ctime>

namespace netkit {

class runloop : public object
{
public:

	typedef void *source;
	
	enum class event
	{
		connect		= ( 1 << 0 ),
		accept		= ( 1 << 1 ),
		read		= ( 1 << 2 ),
		write		= ( 1 << 3 ),
		oob			= ( 1 << 4 ),
		timer		= ( 1 << 5 )
	};
	
	typedef smart_ptr< runloop > ptr;

	typedef std::function< void ( void ) >				dispatch_f;
	typedef std::function< void ( source s, event e ) >	event_f;

	static runloop::ptr
	instance();
	
	virtual source
	create_source( int fd, event e, event_f f ) = 0;
	
	virtual source
	create_source( std::time_t time, event_f f ) = 0;
	
	virtual void
	schedule( source s ) = 0;
	
	virtual void
	cancel( source s ) = 0;
	
	virtual void
	dispatch_on_main_thread( dispatch_f f ) = 0;

	virtual void
	run() = 0;
	
	virtual void
	stop() = 0;
};

}

inline netkit::runloop::event
operator|( netkit::runloop::event a, netkit::runloop::event b )
{
	typedef std::underlying_type< netkit::runloop::event >::type enum_type;
	return static_cast< netkit::runloop::event >(static_cast<enum_type>( a ) | static_cast<enum_type>( b ) );
}

#endif
