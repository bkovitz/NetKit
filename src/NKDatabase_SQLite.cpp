#include "NKDatabase_SQLite.h"
#include <NetKit/NKUnicode.h>
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
#if defined( UNICODE )
manager::create( const std::wstring &s )
#else
manager::create( const std::string &s )
#endif
{
	sqlite3	*db;
	int		err;
	
#if defined( UNICODE )
	err = sqlite3_open16( s.c_str(), &db );
#else
	err = sqlite3_open( s.c_str(), &db );
#endif

	if ( err == SQLITE_OK )
	{
#if defined( __APPLE__ )

		chmod( s.c_str(), 0666 );
	
#endif

		g_manager = new manager_impl( db );
	}
	else
	{
#if defined( WIN32 )
		nklog( log::error, "sqlite3_open() failed: code = %d, message = %s", err, sqlite3_errstr( err ) );
#else
		nklog( log::error, "sqlite3_open() failed: code = %d", err );
#endif
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
	m_db( db ),
	m_ignore_changes( false )
{
	sqlite3_update_hook( m_db, database_was_changed, this );
}


manager_impl::~manager_impl()
{
	sqlite3_close( m_db );

	for ( auto it1 = m_omap.begin(); it1 != m_omap.end(); it1++ )
	{
		if ( it1 != m_omap.end() )
		{
			for ( auto it2 = it1->second.begin(); it2 != it1->second.end(); it2++ )
			{
				( *it2 )->invalidate();
			} 
		}
	}
}


bool
manager_impl::is_connected() const
{
	return ( m_db != NULL ) ? true : false;
}


netkit::status
manager_impl::exec( const std::string &str, bool quiet )
{
	char *error = NULL;
	
	if ( !quiet )
	{
		nklog( log::voluminous, "exec: %s", str.c_str() );
	}

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

	nklog( log::voluminous, "%s\n", str.c_str() );

	return ( sqlite3_prepare_v2( m_db, str.c_str(), -1, &stmt, NULL ) == SQLITE_OK ) ? new statement_impl( stmt ) : new statement_impl( NULL );
}


std::uint32_t
database::manager_impl::version() const
{
	static sqlite3_stmt	*stmt;
	std::uint32_t		version = 0;

	if ( sqlite3_prepare_v2( m_db, "PRAGMA user_version;", -1, &stmt, nullptr ) == SQLITE_OK )
	{
		while ( sqlite3_step( stmt ) == SQLITE_ROW )
		{
			version = sqlite3_column_int( stmt, 0 );
        }

		sqlite3_finalize( stmt );
	}

	return version;
}


void
database::manager_impl::set_version( std::uint32_t version )
{
	char				*error = nullptr;
	std::ostringstream	os;

	os << "PRAGMA user_version = " << version << ";";

	int ret = sqlite3_exec( m_db, os.str().c_str(), 0, 0, &error );
	
	if ( ret )
	{
		nklog( log::error, "sqlite3_exec() failed: %d, %s", ret, error );
	}
}


bool
database::manager_impl::backup( const std::string &dest, backup_hook_f hook )
{
	sqlite3 *dest_db;
	bool	ok = false;

	auto err = sqlite3_open( dest.c_str(), &dest_db );

	if ( err )
	{
		nklog( log::error, "sqlite3_open('%s') failed: %d", dest.c_str(), err );
		ok = false;
		goto exit;
	}

	if ( hook )
	{
		std::string s = hook();

		char *error = NULL;
	
		int ret = sqlite3_exec( dest_db, s.c_str(), 0, 0, &error );
	
		if ( ret )
		{
			nklog( log::error, "sqlite3_exec() failed: %d, %s", ret, error );
			ok = false;
			goto exit;
		}
	}

	auto backup = sqlite3_backup_init( dest_db, "main", m_db, "main" );

	if ( !backup )
	{
		nklog( log::error, "sqlite3_backup_init failed: %d", sqlite3_errcode( dest_db ) );
		ok = false;
		goto exit;
    }

	sqlite3_backup_step( backup, -1 );
	sqlite3_backup_finish( backup );
	sqlite3_close( dest_db );

	ok = true;

exit:

	return ok;
}


bool
database::manager_impl::restore( const std::string &from, restore_hook_f hook )
{
	std::string filename;
	auto		raw	= sqlite3_db_filename( m_db, "main" );
	auto		ok	= false;

	if ( !raw )
	{
		nklog( log::error, "unable to get backing filename" );
		ok = false;
		goto exit;
	}

	filename = raw;

	auto err = sqlite3_close( m_db );

	if ( err )
	{
		nklog( log::error, "sqlite3_close() failed: %d", err );
		ok = false;
		goto exit;
	}

	if ( !platform::copy_file( from, filename ) )
	{
		nklog( log::error, "unable to copy file" );
		ok = false;
		goto exit;
	}

	err = sqlite3_open( filename.c_str(), &m_db );

	if ( err )
	{
		nklog( log::error, "sqlite3_open('%s') failed: %d", filename.c_str(), err );
		ok = false;
		goto exit;
	}

	if ( hook )
	{
		std::string s = hook();

		char *error = NULL;
	
		int ret = sqlite3_exec( m_db, s.c_str(), 0, 0, &error );
	
		if ( ret )
		{
			nklog( log::error, "sqlite3_exec() failed: %d, %s", ret, error );
			ok = false;
			goto exit;
		}
	}

	ok = true;

exit:

	return ok;
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
database::manager_impl::on_change( netkit::cookie::ref *cookie, const std::string &table_name, observer_reply_f reply )
{
	auto		it		= m_omap.find( table_name );
	oids		oids;
	cookie::ref	ret;

	auto o = new observer( table_name, reply, [=]( netkit::cookie::naked_ptr p ) mutable
	{
		if ( p->is_valid() )
		{
			auto it1 = m_omap.find( table_name );
	
			if ( it1 != m_omap.end() )
			{
				for ( auto it2 = it1->second.begin(); it2 != it1->second.end(); it2++ )
				{
					if ( *it2 == p )
					{
						it1->second.erase( it2 );
						break;
					}
				} 
			}
		}
	} );
	
	if ( it != m_omap.end() )
	{
		it->second.push_back( o );
	}
	else
	{
		list l;

		l.push_back( o );
		
		m_omap[ table_name ] = l;
	}
	
	auto stmt = select( "SELECT oid from " + table_name );
		
	while ( stmt->step() )
	{
		oids.push_back( stmt->int64_at_column( 0 ) );
	}

	if ( oids.size() )
	{
		reply( database::action::update, oids );
	}

	if ( cookie )
	{
		cookie->reset( o );
	}
}


void
database::manager_impl::set_ignore_changes( bool val )
{
	m_ignore_changes = val;
}
		

void
database::manager_impl::database_was_changed( void* context, int action, const char* db_name, const char* table_name, sqlite_int64 oid )
{
	database::manager_impl	*self	= ( database::manager_impl* ) context;

	if ( !self->m_ignore_changes )
	{
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

