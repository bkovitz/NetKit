#ifndef _netkit_runloop_win32_h
#define _netkit_runloop_win32_h

#include	<NetKit/NKRunLoop.h>
#include	<NetKit/NKConcurrent.h>
#include	<WinSock2.h>
#include	<WS2tcpip.h>
#include	<IPHlpApi.h>
#include	<mswsock.h>
#include	<functional>
#include	<utility>
#include	<queue>
#include	<mutex>
#include	<list>

class runloop_win32 : public netkit::runloop
{
public:

	class fd_win32 : public netkit::runloop::fd
	{
	public:

		struct context : public WSAOVERLAPPED
		{
			enum type
			{
				iocp_connect	= 1,
				iocp_accept		= 2,
				iocp_send		= 3,
				iocp_recv		= 4,
				iocp_recvfrom	= 5
			};

			context( type val )
			{
				ZeroMemory( this, sizeof( context ) );
				m_type = val;
			}

			type m_type;
		};

		struct connect_context : public context
		{
			connect_context()
			:
				context( iocp_connect ),
				m_reply( nullptr )
			{
			}

			connect_reply_f			m_reply;
			netkit::endpoint::ref	m_to;
		};

		struct accept_context : public context
		{
			accept_context()
			:
				context( iocp_accept ),
				m_reply( nullptr )
			{
			}

			accept_reply_f	m_reply;
			SOCKET			m_fd;
		};

		struct send_context : public context
		{
			send_context()
			:
				context( iocp_send ),
				m_reply( nullptr )
			{
			}

			send_reply_f m_reply;
		};

		struct recv_context : public context
		{
			recv_context()
			:
				context( iocp_recv ),
				m_reply( nullptr )
			{
			}

			DWORD			m_flags;
			recv_reply_f	m_reply;
		};

		struct recvfrom_context : public context
		{
			recvfrom_context()
			:
				context( iocp_recvfrom ),
				m_reply( nullptr )
			{
			}

			sockaddr_storage	m_addr;
			INT					m_addr_len;
			DWORD				m_flags;
			recvfrom_reply_f	m_reply;
		};

		fd_win32( SOCKET fd, int domain, HANDLE port );

		virtual ~fd_win32();

		virtual int
		bind( netkit::endpoint::ref to );

		virtual void
		connect( netkit::endpoint::ref to, connect_reply_f reply );

		void
		handle_connect( int status );

		virtual void
		accept( accept_reply_f reply );

		void
		handle_accept( int status );

		virtual void
		send( const std::uint8_t *buf, std::size_t len, send_reply_f reply );

		virtual void
		sendto( const std::uint8_t *buf, std::size_t len, netkit::endpoint::ref to, send_reply_f reply );

		void
		handle_send( int status, send_context *context );

		virtual void
		recv( recv_reply_f reply );

		void
		handle_recv( int status, DWORD bytes_read );

		virtual void
		recvfrom( recvfrom_reply_f reply );

		void
		handle_recvfrom( int status, DWORD bytes_read );

		virtual void
		close();

	private:

		static LPFN_CONNECTEX				m_connect_ex;
		static LPFN_ACCEPTEX				m_accept_ex;
		static LPFN_GETACCEPTEXSOCKADDRS	m_get_accept_sockaddrs;

		connect_context						m_connect_context;
		accept_context						m_accept_context;
		recv_context						m_recv_context;
		recvfrom_context					m_recvfrom_context;
		std::vector< std::uint8_t >			m_in_buf;
		int									m_domain;
		HANDLE								m_port;
		SOCKET								m_fd;
	};

	static runloop_win32*
	main();

	runloop_win32();

	virtual ~runloop_win32();

	virtual fd::ref
	create( std::int32_t domain, std::int32_t type, std::int32_t protocol );

	virtual fd::ref
	create( netkit::endpoint::ref in_endpoint, netkit::endpoint::ref &out_endpoint, std::int32_t domain, std::int32_t type, std::int32_t protocol );

	virtual event
	create( HANDLE handle );
	
	virtual event
	create( std::time_t msec );
	
	virtual void
	schedule( event e, event_f func );
	
	virtual void
	schedule_oneshot_timer( std::time_t msec, event_f func );

	virtual void
	suspend( event e );

	virtual void
	cancel( event e );
	
	virtual void
	dispatch( dispatch_f f );

	virtual void
	run( mode how = mode::normal );

	virtual void
	stop();
	
private:

	struct source
	{
		typedef std::vector< source* >	vector;
		typedef std::list< source* >	list;

		static inline bool
		compare( source *lhs, source *rhs )
		{
			return lhs->m_absolute_time < rhs->m_absolute_time;
		}

		HANDLE			m_handle;
		std::time_t		m_relative_time;
		std::time_t		m_absolute_time;
		bool			m_scheduled;
		bool			m_oneshot;
		event_f			m_func;
	
		source();

		~source();

		inline bool
		is_handle() const
		{
			return ( m_handle != WSA_INVALID_EVENT ) ? true : false;
		}

		inline bool
		is_timer() const
		{
			return ( m_handle == WSA_INVALID_EVENT ) ? true : false;
		}

		void
		dispatch();
	};

	bool			m_setup;
	source::vector	m_sources;
	source::list	m_timers;
	bool			m_running;

	typedef netkit::concurrent::queue< std::pair< void*, dispatch_f > >		queue;

	HANDLE			m_iocp_thread;
	HANDLE			m_wakeup;
	HANDLE			m_port;
	HANDLE			m_network;
	DWORD			m_result;

	queue					m_queue;
	std::recursive_mutex	m_mutex;
		
	bool
	init();

	bool
	running()
	{
		std::lock_guard< std::recursive_mutex > guard( m_mutex );
		return m_running;
	}

	void
	set_running( bool val )
	{
		std::lock_guard< std::recursive_mutex > guard( m_mutex );
		m_running = val;
	}

	inline void
	push( dispatch_f f )
	{
		push( NULL, f );
	}

	inline void
	push( void *context, dispatch_f f )
	{
		m_queue.push( std::make_pair( context, f ) );
		SetEvent( m_wakeup );
	}
		
	void
	schedule( source *s );

	void
	suspend( source *s );

	void
	run( mode how, bool &input_event );
	
};


#endif
