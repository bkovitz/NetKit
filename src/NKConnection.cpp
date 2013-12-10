#include <NetKit/NKConnection.h>

#if 0
using namespace netkit;

connection::connection()
:
	m_disconnect_handler( NULL )
{
	init();
}


connection::~connection()
{
}


bool
connection::flush()
{
	std::string msg = m_ostream.str();
	ssize_t		num = 0;
		
	if ( msg.size() > 0 )
	{
		nklog( log::verbose, "sending msg: %s", msg.c_str() );
		//num = m_socket->send( reinterpret_cast< const uint8_t* >( msg.c_str() ), msg.size() );
		m_ostream.str( "" );
		m_ostream.clear();
	}
		
	return ( msg.size() == num ) ? true : false;
}


ssize_t
connection::recv( uint8_t *buf, size_t len )
{
	//	int num = ( int ) m_socket->recv( buf, len );
	int num ;
		
	if ( num <= 0 )
	{
		close();
	}
		
	return num;
}


ssize_t
connection::peek( uint8_t *buf, size_t len )
{
//		return m_socket->peek( buf, len );
	return 0;
}


void
connection::init()
{
}


void
connection::close()
{
}


#endif