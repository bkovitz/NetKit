#ifndef _netkit_connection_h
#define _netkit_connection_h

#include <NetKit/NKSink.h>
#include <NetKit/NKSource.h>
#include <NetKit/NKLog.h>
#include <functional>
#include <sstream>

namespace netkit {

class connection : public sink
{
public:

	typedef std::function< void ( void ) > disconnect_handler;

public:

	typedef smart_ptr< connection > ptr;
	
	connection();
	
	virtual ~connection();
	
	inline void
	set_disconnect_handler( disconnect_handler handler )
	{
		m_disconnect_handler = handler;
	}
	
	template < class U >
	inline connection&
	operator<<( U t )
	{
		m_ostream << t;
		return *this;
	}
	
	inline connection&
	operator<<( connection& ( *func )( connection& ) )
	{
		return func( *this );
	}
	
	inline void
	write( const uint8_t *buf, size_t len )
	{
		m_ostream.write( reinterpret_cast< const char* >( buf ), len );
	}
	
	bool
	flush();
	
	ssize_t
	recv( uint8_t *buf, size_t len );
	
	ssize_t
	peek( uint8_t *buf, size_t len );
	
	void
	close();

protected:

	virtual void
	can_send_data() = 0;

	virtual void
	can_recv_data() = 0;

private:

	void
	init();
	
	std::ostringstream	m_ostream;
	disconnect_handler	m_disconnect_handler;
};

/*
inline connection< class T >
flush( connection< class T> &obj )
{
	obj.drain();
	return obj;
}
*/

}

#endif