#ifndef _CoreApp_connection_h
#define _CoreApp_connection_h

#include <CoreApp/tcp_socket.h>
#include <CoreApp/dispatch.h>
#include <CoreApp/log.h>
#include <functional>
#include <sstream>

namespace CoreApp {

template < class T >
class connection : public object
{
public:

	typedef std::function< void ( void ) > disconnect_handler;

public:

	typedef smart_ptr< connection > ptr;
	
	typedef smart_ptr< T > T_ptr;
	
	connection()
	:
		m_socket( new T ),
		m_disconnect_handler( NULL )
	{
		init();
	}

	connection( const T_ptr& socket )
	:
		m_socket( socket ),
		m_disconnect_handler( NULL )
	{
		init();
	}

	virtual ~connection()
	{
	}
	
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
	
	inline bool
	flush()
	{
		std::string msg = m_ostream.str();
		ssize_t		num = 0;
		
		if ( msg.size() > 0 )
		{
			netlog( log::verbose, "sending msg: %s", msg.c_str() );
			num = m_socket->send( reinterpret_cast< const uint8_t* >( msg.c_str() ), msg.size() );
			m_ostream.str( "" );
			m_ostream.clear();
		}
		
		return ( msg.size() == num ) ? true : false;
	}
	
	inline int
	recv( uint8_t *buf, size_t len )
	{
		int num = ( int ) m_socket->recv( buf, len );
		
		if ( num <= 0 )
		{
			close();
		}
		
		return num;
	}
	
	inline int
	peek( uint8_t *buf, size_t len )
	{
		return m_socket->peek( buf, len );
	}
	
	inline void
	close()
	{
		if ( m_socket->fd() != socket::null )
		{
			m_socket->close();
			
			if ( m_disconnect_handler )
			{
				m_disconnect_handler();
			}
		}
	}

protected:

	virtual void
	can_send_data() = 0;

	virtual void
	can_recv_data() = 0;

	T_ptr m_socket;
	
private:

	inline void
	init()
	{
		m_socket->set_recv_handler( [this]()
		{
			can_recv_data();
		} );
	}
	
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