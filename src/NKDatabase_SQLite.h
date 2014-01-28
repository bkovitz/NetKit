//
//  database_impl.h
//  FingerPrint
//
//  Created by Scott Herscher on 2/6/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef _netkit_database_impl_h
#define _netkit_database_impl_h

#include <NetKit/NKDatabase.h>
#include <sqlite3.h>
#include <list>
#include <map>

namespace netkit {

namespace database {

class statement_impl : public statement
{
public:

	typedef smart_ref< statement > ref;

	statement_impl( sqlite3_stmt *stmt )
	:
		m_stmt( stmt )
	{
	}

	~statement_impl()
	{
		if ( m_stmt )
		{
			sqlite3_finalize( m_stmt );
		}
	}

	bool
	step()
	{
		return ( m_stmt ) ? ( sqlite3_step( m_stmt ) == SQLITE_ROW ) ? true : false : false;
	}

	bool
	bool_at_column( int col ) const
	{
		return ( m_stmt ) ? ( sqlite3_column_int( m_stmt, col ) ? true : false ) : false;
	}

	int
	int_at_column( int col ) const
	{
		return ( m_stmt ) ? ( int ) sqlite3_column_int( m_stmt, col ) : 0;
	}

	int64_t
	int64_at_column( int col ) const
	{
		return ( m_stmt ) ? ( int64_t ) sqlite3_column_int64( m_stmt, col ) : 0;
	}

	std::string
	text_at_column( int col ) const
	{
		return ( m_stmt ) ? ( char* ) sqlite3_column_text( m_stmt, col ) : "";  
	}

private:

	sqlite3_stmt *m_stmt;
};


class manager_impl : public manager
{
public:

	manager_impl( sqlite3 *db );
	
	virtual
	~manager_impl();
	
	virtual bool
	is_connected() const;
	
	virtual cookie::ref
	on_change( const std::string &tableName, observer_reply_f reply );
	
	virtual void
	set_ignore_changes( bool val );

	virtual netkit::status
	exec( const std::string &str, bool quiet = false );

	virtual statement::ref
	select( const std::string &str );
	
	virtual int64_t
	last_row_id()
	{
		return sqlite3_last_insert_rowid( m_db );
	}

	virtual std::uint32_t
	version() const;

	virtual void
	set_version( std::uint32_t version );

	bool
	close();

private:

	class observer : public netkit::cookie
	{
	public:

		observer( const std::string &table_name, observer_reply_f reply )
		:
			m_table_name( table_name ),
			m_reply( reply )
		{
		}

		inline const std::string&
		table_name() const
		{
			return m_table_name;
		}

		inline observer_reply_f&
		reply()
		{
			return m_reply;
		}

	private:

		std::string			m_table_name;
		observer_reply_f	m_reply;
	};
		
	typedef std::list< observer* >			list;
	typedef std::map< std::string, list >	map;
	
	static void
	database_was_changed( void* impl, int, const char* db_name, const char* table_name, sqlite_int64 );

	bool	m_ignore_changes;
	map		m_omap;
	sqlite3	*m_db;
};

}

}

#endif
