#include "NKRunLoop_Win32.h"
#include <NetKit/NKLog.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <process.h>
#include <list>

/*
 * runloop_win32 Methods
 */

static runloop_win32 *g_instance;

using namespace netkit;

static void
ShiftDown( void * arr, size_t arraySize, size_t itemSize, int index )
{
    memmove( ( ( unsigned char* ) arr ) + ( ( index - 1 ) * itemSize ), ( ( unsigned char* ) arr ) + ( index * itemSize ), ( arraySize - index ) * itemSize );
}


runloop::ptr
runloop::instance()
{
    static runloop::ptr singleton = new runloop_win32;

    return singleton;
}


runloop_win32*
runloop_win32::instance()
{
	return g_instance;
}


runloop_win32::runloop_win32()
:
	m_running( FALSE )
{
	// We should check the retrn value of this

	g_instance = this;

	m_main.init();
}


runloop_win32::~runloop_win32()
{
}


runloop::event
runloop_win32::create( int fd, event_mask m )
{
	source		*s	= nullptr;
	atom		*a	= nullptr;
	DWORD		err	= 0;

	fprintf( stderr, "in runloop create for fd: %d with event mask %d\n", fd, m );
	for ( auto it = m_sources.begin(); it != m_sources.end(); it++ )
	{
		if ( ( *it )->m_socket == fd )
		{
			s = *it;
			break;
		}
	}

	if ( !s )
	{
		try
		{
			s = new source;
		}
		catch ( ... )
		{
			s = nullptr;
		}

		if ( !s )
		{
			err = -1;
			goto exit;
		}

		s->m_socket = fd;
		s->m_handle = WSACreateEvent();

		if ( s->m_handle == WSA_INVALID_EVENT )
		{
			err = -1;
			goto exit;
		}

		m_sources.push_back( s );
	}
	
	try
	{
		a = new atom;
	}
	catch ( ... )
	{
		err = -1;
		goto exit;
	}

	if ( m & runloop::event_mask::read )
	{
		a->m_network_events |= ( FD_ACCEPT | FD_READ | FD_CLOSE );
	}
	
	if ( m & runloop::event_mask::write )
	{
		a->m_network_events |= ( FD_CONNECT | FD_WRITE );
	}

	a->m_source = s;
	s->m_atoms.push_back( a );

exit:

	if ( err != 0 )
	{
		if ( s != nullptr )
		{
			delete s;
			s = nullptr;
		}

		if ( a != nullptr )
		{
			delete a;
			a = nullptr;
		}
	}

	return a;
}


runloop::event
runloop_win32::create( HANDLE handle )
{
	source	*s = nullptr;
	atom	*a = nullptr;
	
	try
	{
		s = new source;
		a = new atom;
	}
	catch ( ... )
	{
		s = nullptr;
		a = nullptr;
	}

	if ( !s || !a )
	{
		goto exit;
	}

	a->m_source = s;

	s->m_handle	= handle;
	s->m_atoms.push_back( a );

exit:

	return s;
}


runloop::event
runloop_win32::create( std::time_t msec )
{
	return nullptr;
}


void
runloop_win32::modify( event e, std::time_t msec )
{
}

	
void
runloop_win32::schedule( event e, event_f func )
{
	atom	*a			= reinterpret_cast< atom* >( e );
	source	*s			= a->m_source;
	worker	*w			= nullptr;
	DWORD	registered	= FALSE;
	DWORD	err			= 0;

	fprintf( stderr, "in runloop schdule for fd: %d\n", s->m_socket );

	// First check our main Worker. In most cases, we won't have to worry about threads

	if ( a->m_scheduled )
	{
		nklog( log::error, "trying to schedule an event that has already been scheduled on fd %d", s->m_socket );
		goto exit;
	}

	a->m_scheduled	= true;
	a->m_func		= func;

	if ( s->is_socket() )
	{
		err = WSAEventSelect( s->m_socket, s->m_handle, s->network_events() );

		if ( err )
		{
			nklog( log::error, "WSAEventSelect() failed with error code: %d", ::GetLastError() );
			a->m_scheduled = false;
			goto exit;
		}
	}

	if ( !s->m_scheduled )
	{
		if ( m_sources.size() < MAXIMUM_WAIT_OBJECTS )
		{
			m_main.schedule( s );
		}
		else
		{
			BOOL registered = FALSE;
	
			// Try to find a thread to use that we've already created
	
			for ( worker::list::iterator it = m_workers.begin(); it != m_workers.end(); it++ )
			{
				if ( ( *it )->m_num_sources < MAXIMUM_WAIT_OBJECTS )
				{
					worker *w = *it;

					w->push( s, [=]()
					{
						w->schedule( s );
					} );
	
					registered = TRUE;
					break;
				}
			}
	
			// If not, then create a Worker and make a thread to run it in
			
			if ( !registered )
			{
				try
				{
					w = new worker;
				}
				catch ( ... )
				{
					w = nullptr;
				}
	
				if ( !w )
				{
					err = -1;
					goto exit;
				}
	
				if ( !w->init() )
				{
					err = -1;
					goto exit;
				}
			
				w->schedule( s );
	
				w->m_thread = ( HANDLE ) _beginthreadex( nullptr, 0, worker::main, w, 0, nullptr );

				if ( w->m_thread == nullptr )
				{
					err = GetLastError();
					goto exit;
				}
	
				m_workers.push_back( w );
			}
		}
	}

exit:

	if ( err && w )
	{
		delete w;
	}

	return;
}


/*
void
runloop_win32::unregisterSocket( socket::ptr sock )
{	
	for ( source::list::iterator it = m_sources.begin(); it != m_sources.end(); it++ )
	{
		if ( ( ( *it )->m_type == source::socketType ) && ( ( *it )->m_socket->fd() == sock->fd() ) )
		{
			source *s = *it;
			
			WSAEventSelect( sock->fd(), nullptr, 0 );
			WSACloseEvent( s->m_handle );
			unregisterSource( s );
			delete s;
			break;
		}
	}
}	
*/

void
runloop_win32::suspend( event e )
{
	atom	*a = reinterpret_cast< atom* >( e );
	source	*s = a->m_source;

	if ( !a->m_scheduled )
	{
		nklog( log::error, "trying to suspend an event that has not been scheduled" );
		goto exit;
	}

	a->m_scheduled = false;

	if ( s->is_socket() )
	{
		int err = WSAEventSelect( s->m_socket, s->m_handle, s->network_events() );

		if ( err )
		{
			nklog( log::error, "WSAEventSelect() failed with error code: %d", ::GetLastError() );
		}
	}

	if ( ( s->is_handle() ) || ( s->network_events() == 0 ) )
	{
		if ( s->m_owner == &m_main )
		{
			s->m_owner->suspend( s );
		}
		else
		{
			HANDLE	sync = CreateEvent( nullptr, FALSE, FALSE, nullptr );
			DWORD	err;

			s->m_owner->push( s, [=]()
			{
				s->m_owner->suspend( s );
				SetEvent( sync );
			} );
	
			err = WaitForSingleObject( sync, 5 * 1000 );
	
			if ( err == WAIT_FAILED )
			{
				nklog( log::error, "error while waiting to synchronize remove source: %d", GetLastError() );
			}
			else if ( err == WAIT_TIMEOUT )
			{
				nklog( log::error, "timed out waiting to synchronize remove source" );
			}
	
			m_main.m_queue.prune( [=]( std::pair< void*, dispatch_f > &item )
			{
				return ( item.first == s ) ? true : false;
			} );
	
			CloseHandle( sync );
		}
	}

exit:

	return;
}

	
void
runloop_win32::cancel( event e )
{
	atom	*a = reinterpret_cast< atom* >( e );
	source	*s = a->m_source;

	suspend( e );

	s->m_atoms.remove( a );

	// Don't delete the atom now, because it holds the lambda context.  This will allow us
	// to call cancel while in the context of a lambda handler.  Otherwise, we'd be deleting
	// the context out from under us
	//
	// We'll perform a "neat" trick of delaying the delete until later...

	dispatch_on_main_thread( [=]() mutable
	{
		delete a;

		if ( s->m_atoms.size() == 0 )
		{
			m_sources.remove( s );
			delete s;
		}
	} );
}

	
void
runloop_win32::dispatch_on_main_thread( dispatch_f f )
{
	m_main.push( f );
}


void
runloop_win32::run()
{
	m_running = TRUE;

	while ( m_running )
	{
		m_main.run( INFINITE );
	}
}


void
runloop_win32::stop()
{
	m_running = FALSE;
}


runloop_win32::source::source()
:
	m_socket( INVALID_SOCKET ),
	m_handle( WSA_INVALID_EVENT ),
	m_scheduled( false ),
	m_owner( nullptr )
{
}


runloop_win32::source::~source()
{
	if ( m_socket != INVALID_SOCKET )
	{
		WSAEventSelect( m_socket, nullptr, 0 );
	}

	if( m_handle != WSA_INVALID_EVENT )
	{
		WSACloseEvent( m_handle );
	}
}


void
runloop_win32::source::dispatch()
{
	atom *a = nullptr;

	assert( m_scheduled );
	assert( m_atoms.size() > 0 );

	if ( is_socket() )
	{
		WSANETWORKEVENTS network_events;

		if ( WSAEnumNetworkEvents( m_socket, m_handle, &network_events ) == 0 )
		{
			for ( auto it = m_atoms.begin(); it != m_atoms.end(); it++ )
			{
				if ( network_events.lNetworkEvents & ( *it )->m_network_events )
				{
					a = ( *it );
					break;
				}
			}
		}

		if ( !a )
		{
			for ( auto it = m_atoms.begin(); it != m_atoms.end(); it++ )
			{
				if ( ( *it )->m_scheduled )
				{
					a = ( *it );
					break;
				}
			}
		}
	}
	else
	{
		assert( m_atoms.size() == 1 );
		a = m_atoms.front();
	}

	if ( a )
	{
		a->m_func( a );
	}
	else
	{
		nklog( log::error, "got a dispatch request but cannot lookup the atom" );
		assert( 0 );
	}
}


runloop_win32::atom::atom()
:
	m_network_events( 0 ),
	m_scheduled( false ),
	m_source( nullptr )
{
}


runloop_win32::atom::~atom()
{
}


unsigned __stdcall
runloop_win32::worker::main( void * arg )
{
	worker *self = reinterpret_cast< worker* >( arg );

	while ( self->running() )
	{
		self->run( INFINITE );
	}

	return 0;
}


runloop_win32::worker*
runloop_win32::worker::get_main_worker()
{
	return &runloop_win32::instance()->m_main;
}


runloop_win32::worker::worker()
:
	m_id( 0 ),
	m_wakeup( INVALID_HANDLE_VALUE ),
	m_thread( INVALID_HANDLE_VALUE ),
	m_done( FALSE ),
	m_num_sources( 0 ),
	m_result( 0 ),
	m_running( true )
{
}


runloop_win32::worker::~worker()
{
	if ( m_wakeup )
	{
		CloseHandle( m_wakeup );
		m_wakeup = INVALID_HANDLE_VALUE;
	}
}


bool
runloop_win32::worker::init()
{
	atom	*a;
	source	*s;
	DWORD	err = 0;

	try
	{
		a = new atom;
		s = new source;
	}
	catch ( ... )
	{
		a = nullptr;
		s = nullptr;
	}

	if ( !a || !s )
	{
		err = -1;
		goto exit;
	}
	
	a->m_func = [=]( event e )
	{
		std::pair< void*, dispatch_f > item;

		while ( m_queue.try_pop( item ) )
		{
			item.second();
		}
	};

	a->m_source = s;
	s->m_handle	= CreateEvent( nullptr, FALSE, FALSE, nullptr );
	s->m_atoms.push_back( a );
	m_wakeup = s->m_handle;
	
	schedule( s );

exit:

	return ( err == 0 ) ? true : false;
}


void
runloop_win32::worker::schedule( source *s )
{
	s->m_scheduled	= true;
	s->m_owner		= this;

	m_sources[ m_num_sources ] = s;
	m_handles[ m_num_sources ] = s->m_handle;

	m_num_sources++;
}


int
runloop_win32::worker::source_to_index( source *s )
{
	int index;

	for ( index = 0; index < ( int ) m_num_sources; index++ )
	{
		if ( m_sources[ index ] == s )
		{
			break;
		}
	}

	if ( index == ( int ) m_num_sources )
	{
		index = -1;
	}

	return index;
}


void
runloop_win32::worker::suspend( source *s )
{
	int		source_index = source_to_index( s );
	DWORD	delta;

	if ( source_index == -1 )
	{
		nklog( log::error, "source not found in list" );
		goto exit;
	}

	delta = ( m_num_sources - source_index - 1 );

	// If this Source is not at the end of the list, then move memory

	if ( delta > 0 )
	{
		ShiftDown( m_sources, m_num_sources, sizeof( m_sources[ 0 ] ), source_index + 1 );
		ShiftDown( m_handles, m_num_sources, sizeof( m_handles[ 0 ] ), source_index + 1 );
	}
		         
	m_num_sources--;

	s->m_scheduled = false;

exit:

	return;
}


void
runloop_win32::worker::run( int32_t msec )
{
	DWORD result;
	DWORD err = 0;

	result = WaitForMultipleObjects( m_num_sources, m_handles, FALSE, INFINITE );

	if ( result == WAIT_FAILED )
	{
		err = GetLastError();
	}

	if ( err )
	{
		nklog( log::error, "WaitForMultipleObjects() returned error: %d", err );
		goto exit;
	}

	if ( result == WAIT_TIMEOUT )
	{
		nklog( log::verbose, "timeout" );
	}
	else
	{
		DWORD	waitItemIndex = ( DWORD )( ( ( int ) result ) - WAIT_OBJECT_0 );
		source	*s = nullptr;
		
		// Sanity check

		if ( waitItemIndex >= m_num_sources )
		{
			nklog( log::error, "waitItemIndex (%d) is >= numSources (%d)", waitItemIndex, m_num_sources );
			goto exit;
		}

		s = m_sources[ waitItemIndex ];

		if ( ( get_main_worker() == this ) || ( waitItemIndex == 0 ) )
		{
			fprintf( stderr, "dispatching event to handler\n" );
			s->dispatch();
		}
		else
		{
			get_main_worker()->push( s, [=]()
			{
				s->dispatch();
			} );
		}
	}

exit:

	return;
}