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

	static runloop_win32*
	instance();

	runloop_win32();

	virtual ~runloop_win32();

	virtual event
	create( int fd, event_mask m );

	virtual event
	create( HANDLE handle );
	
	virtual event
	create( std::time_t msec );
	
	virtual void
	modify( event e, std::time_t msec );
	
	virtual void
	schedule( event e, event_f func );
	
	virtual void
	suspend( event e );
	
	virtual void
	cancel( event e );
	
	virtual void
	dispatch_on_main_thread( dispatch_f f );

	virtual void
	run();

	virtual void
	stop();
	
private:

	struct source;
	struct worker;

	struct atom
	{
		typedef std::list< atom* > list;

		static inline bool
		compare( atom *lhs, atom *rhs )
		{
			return lhs->m_absolute_time < rhs->m_absolute_time;
		}

		long			m_network_events;
		std::time_t		m_relative_time;
		std::time_t		m_absolute_time;
		bool			m_scheduled;
		source			*m_source;
		event_f			m_func;

		atom();

		~atom();
	};

	struct source
	{
		typedef std::list< source* > list;

		SOCKET			m_socket;
		HANDLE			m_handle;
		worker			*m_owner;
		atom::list		m_atoms;
		bool			m_scheduled;
	
		source();

		~source();

		inline bool
		is_socket() const
		{
			return ( m_socket != INVALID_SOCKET ) ? true : false;
		}

		inline bool
		is_handle() const
		{
			return ( m_socket == INVALID_SOCKET ) ? true : false;
		}

		inline bool
		is_timer() const
		{
			return ( ( m_socket == INVALID_SOCKET ) && ( m_handle == WSA_INVALID_EVENT ) );
		}

		inline long
		network_events()
		{
			long ret = 0;

			for ( auto it = m_atoms.begin(); it != m_atoms.end(); it++ )
			{
				if ( ( *it )->m_scheduled )
				{
					ret |= ( *it )->m_network_events;
				}
			}

			return ret;
		}

		void
		dispatch();
	};

	struct worker
	{
		typedef std::list< atom* >												timers;
		typedef netkit::concurrent::queue< std::pair< void*, dispatch_f > >		queue;
		typedef std::list< worker* >											list;

		HANDLE		m_thread;
		unsigned	m_id;			// 0 for main worker

		HANDLE		m_wakeup;	// NULL for main worker
		BOOL		m_done;		// Not used for main worker

		timers		m_timers;
		DWORD		m_num_sources;
		source		*m_sources[ MAXIMUM_WAIT_OBJECTS ];
		HANDLE		m_handles[ MAXIMUM_WAIT_OBJECTS ];
		DWORD		m_result;

		queue					m_queue;
		std::recursive_mutex	m_mutex;
		bool					m_running;
		
		static unsigned __stdcall
		main( void * arg );

		static worker*
		get_main_worker();

		worker();

		~worker();

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
			std::lock_guard<std::recursive_mutex> guard( m_mutex );
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

		int
		source_to_index( source *s );
		
		void
		suspend( source *s );

		void
		run( int32_t msec );
	};

	BOOL			m_setup;
	source::list	m_sources;
	worker			m_main;
	worker::list	m_workers;
	BOOL			m_running;
	
	void
	remove_worker( worker *w );

public:

	inline worker&
	main_worker()
	{
		return m_main;
	}
};


#endif