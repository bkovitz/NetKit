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

static manager::ref g_manager;

bool
manager::create( const uri::ref &uri )
{
	sqlite3	*db;
	int		err;
	
	err = sqlite3_open( uri->path().c_str(), &db );

	if ( err == SQLITE_OK )
	{
#if defined( __APPLE__ )

		chmod( ( const char* ) uri->path().c_str(), 0666 );
	
#endif

		g_manager = new manager_impl( db );
	}
	
	return ( err == 0 ) ? true : false;
}


manager::ref
manager::instance()
{
	return g_manager;
}


manager_impl::manager_impl( sqlite3 * db )
:
	m_db( db )
{
	sqlite3_update_hook( m_db, database_was_changed, this );
}


manager_impl::~manager_impl()
{
	sqlite3_close( m_db );
}


bool
manager_impl::is_connected() const
{
	return ( m_db != NULL ) ? true : false;
}


netkit::status
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


database::statement::ref
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


netkit::cookie
database::manager_impl::on_change( const std::string &tableName, observer_reply_f reply )
{
	auto		it	= m_omap.find( tableName );
	observer	*o	= new observer( tableName, reply );
	oids		oids;
	
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
	
	auto stmt = select( "SELECT oid from " + tableName );
		
	while ( stmt->step() )
	{
		oids.push_back( stmt->int64_at_column( 0 ) );
	}

	if ( oids.size() )
	{
		reply( database::action::update, oids );
	}

	return o;
}
		

void
database::manager_impl::cancel( cookie c )
{
	observer *o = reinterpret_cast< observer* >( c.get() );

	if ( o )
	{
		auto it1 = m_omap.find( o->table_name() );
	
		if ( it1 != m_omap.end() )
		{
			for ( auto it2 = it1->second.begin(); it2 != it1->second.end(); it2++ )
			{
				if ( *it2 == c.get() )
				{
					it1->second.erase( it2 );
				}
			} 
		}

		delete o;
	}
}


void
database::manager_impl::database_was_changed( void* context, int action, const char* db_name, const char* table_name, sqlite_int64 oid )
{
	database::manager_impl	*self	= ( database::manager_impl* ) context;
	map::iterator			it1		= self->m_omap.find( table_name );
	oids					oids;

	oids.push_back( oid );

	if ( it1 != self->m_omap.end() )
	{
		for ( auto it2 = it1->second.begin(); it2 != it1->second.end(); it2++ )
		{
			switch ( action )
			{
				 case SQLITE_INSERT:
				 case SQLITE_UPDATE:
				 {
					( *it2 )->reply()( action::update, oids );
				 }
				 break;
				 
				 case SQLITE_DELETE:
				 {
					( *it2 )->reply()( action::delet, oids );
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

