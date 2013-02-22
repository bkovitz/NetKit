#ifndef _netkit_socket_h
#define _netkit_socket_h

#include <NetKit/NKSource.h>
#include <NetKit/NKSink.h>
#include <NetKit/NKRunLoop.h>
#include <initializer_list>
#include <functional>
#include <errno.h>
#if defined( WIN32 )

#else
#	include <sys/socket.h>
#	include <unistd.h>
#	include <fcntl.h>
#endif

namespace netkit {

namespace socket {

#if defined( WIN32 )

typedef SOCKET native;
static const native null = INVALID_SOCKET;

#else

typedef int native;
static const native null = -1;

#endif

enum class error
{
#if defined( WIN32 )
	in_progress	=	WSAEINPROGRESS,
	would_block	=	WSAEWOULDBLOCK
#else
	in_progress	=	EINPROGRESS,
	would_block	=	EAGAIN
#endif
};


class server : public object
{
public:

	typedef std::function< sink::ptr ( const std::uint8_t *buf, size_t len ) >	adopt_f;
	typedef std::list< adopt_f >												adopters;

	void
	bind( std::initializer_list< adopt_f > l );
	
protected:

	server( int domain, int type );

	server( native fd );
	
	server( const server &that );	// Not implemented
	
	adopters	m_adopters;
	native		m_fd;
};

	
class client : public source
{
public:

	typedef smart_ptr< client > ptr;

	int
	set_blocking( bool block );
	
	virtual void
	close();
	
	inline bool
	is_open() const
	{
		return ( m_fd != null ) ? true : false;
	}
	
	inline native
	fd() const
	{
		return m_fd;
	}
	
protected:

	client( int domain, int type );

	client( native fd );
	
	client( const client &that );	// Not implemented
	
	virtual ~client();

	native		m_fd;
};

}

}

#endif