#ifndef _coreapp_socket_h
#define _coreapp_socket_h

#include <CoreApp/CAObject.h>
#include <CoreApp/CADispatch.h>
#include <functional>
#include <errno.h>

namespace coreapp {

class socket : public object
{
public:

	typedef std::function< void () > handler;

	typedef smart_ptr< socket > ptr;

#if defined( WIN32 )

	typedef SOCKET native;

	enum
	{
		null = INVALID_SOCKET
	};

#else

	typedef int native;

	enum
	{
		null = -1
	};

#endif

	struct error
	{
		enum
		{
	#if defined( WIN32 )
			inprogress	=	WSAEINPROGRESS,
			wouldblock	=	WSAEWOULDBLOCK
	#else
			inprogress	=	EINPROGRESS,
			wouldblock	=	EAGAIN
	#endif
		};
	};
	
	virtual int
	open() = 0;
	
	inline bool
	is_open() const
	{
		return ( m_fd != null ) ? true : false;
	}
	
	void
	set_send_handler( handler h );
	
	void
	set_recv_handler( handler h );
	
	int
	set_blocking( bool block );
	
	virtual void
	close();
	
	inline native
	fd() const
	{
		return m_fd;
	}

protected:

	socket( int domain, int type );
	
	socket( native fd );
	
	socket( const socket &that );	// Not implemented
	
	virtual ~socket() = 0;

	handler				m_send_handler;
	handler				m_recv_handler;
	dispatch_source_t	m_recv_source;
	native				m_fd;
};

}

#endif
