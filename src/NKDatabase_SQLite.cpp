#include "NKDatabase_SQLite.h"
#include <NetKit/NKLog.h>
#include <sstream>

#if defined( __APPLE__ )
#	include <sys/types.h>
#	include <sys/stat.h>
#endif

using namespace netkit;
using namespace netkit::database;

#if defined( __APPLE__ )
#	pragma mark database::manager_impl implementation
#endif

DEFINE_COMPONENT2( manager, manager_impl )

static uri::ptr g_uri;

void
manager::set_uri( const uri::ptr &uri )
{
	g_uri = uri;
}


manager_impl::manager_impl()
:
	m_db( NULL )
{
}


manager_impl::~manager_impl()
{
}


enum status
manager_impl::will_initialize()
{
	if ( status() != status::ok )
	{
		int ret;
	
		ret = sqlite3_open( g_uri->path().c_str(), &m_db );

		if ( ret != SQLITE_OK )
		{
			m_status = status::internal;
			goto exit;
		}
		
#if defined( __APPLE__ )

		chmod( ( const char* ) g_uri->path().c_str(), 0666 );
	
#endif

		m_status = status::ok;
	}

exit:

	return status();
}


enum status
manager_impl::did_initialize()
{
	if ( status() == status::ok )
	{
		for ( map::iterator it = m_omap.begin(); it != m_omap.end(); it++ )
		{
			statement::ptr stmt = select( "SELECT oid from " + it->first );
		
			while ( stmt->step() )
			{				
				database_was_changed( this, SQLITE_INSERT, NULL, it->first.c_str(), stmt->int64_at_column( 0 ) );
			}
		}
		
		sqlite3_update_hook( m_db, database_was_changed, this );
	}

	return status();
}


void
manager_impl::will_terminate()
{
}


enum status
manager_impl::exec( const std::string &str )
{
	char *error = NULL;
	
	nklog( log::verbose, "exec: %s", str.c_str() );

	int ret = sqlite3_exec( m_db, str.c_str(), 0, 0, &error );
	
	if ( ret )
	{
		nklog( log::error, "sqlite3_exec() failed: %d, %s", ret, error );
	}
	
	if ( error )
	{
		sqlite3_free( error );
	}

	return status::ok;
}


database::statement::ptr
database::manager_impl::select( const std::string &str )
{
	sqlite3_stmt *stmt;

	nklog( log::verbose, "select: %s\n", str.c_str() );

	return ( sqlite3_prepare_v2( m_db, str.c_str(), -1, &stmt, NULL ) == SQLITE_OK ) ? new statement_impl( stmt ) : new statement_impl( NULL );
}


bool
database::manager_impl::close()
{
	bool ok = false;
	
	if ( !m_db )
	{
		goto exit;
	}

	if ( sqlite3_close( m_db ) != SQLITE_OK )
	{
		goto exit;
	}
	
	m_db = 0;
	ok = true;
	
exit:

	return ok;
}


void
database::manager_impl::add_observer( const std::string &tableName, observer *o )
{
	map::iterator it = m_omap.find( tableName );
	
	if ( it != m_omap.end() )
	{
		it->second.push_back( o );
	}
	else
	{
		list l;
		
		l.push_back( o );
		
		m_omap[ tableName ] = l;
	}
}


void
database::manager_impl::remove_observer( const std::string &tableName, observer * o )
{
	map::iterator it = m_omap.find( tableName );
	
	if ( it != m_omap.end() )
	{
		it->second.remove( o );
	}
}


void
database::manager_impl::database_was_changed( void* context, int action, const char* db_name, const char* table_name, sqlite_int64 oid )
{
	database::manager_impl	*self	= ( database::manager_impl* ) context;
	map::iterator			it1		= self->m_omap.find( table_name );

	if ( it1 != self->m_omap.end() )
	{
		for ( list::iterator it2 = it1->second.begin(); it2 != it1->second.end(); it2++ )
		{
			switch ( action )
			{
				 case SQLITE_INSERT:
				 case SQLITE_UPDATE:
				 {
					( *it2 )->object_was_updated( table_name, oid );
				 }
				 break;
				 
				 case SQLITE_DELETE:
				 {
					( *it2 )->object_was_removed( table_name, oid );
				 }
				 break;
			}
		}
	}
}


std::string
database::sanitize( const std::string &str )
{
	std::string output;

	for ( std::string::const_iterator it = str.begin(); it != str.end(); it++ )
    {
		if ( *it == '\'' )
		{
			output += "''";
		}
		else
		{
			output += *it;
		}
    }

    return output;
}

