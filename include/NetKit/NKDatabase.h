#ifndef _netkit_database_h
#define _netkit_database_h

#include <NetKit/NKSmartRef.h>
#include <NetKit/NKCookie.h>
#include <NetKit/NKComponent.h>
#include <NetKit/NKJSON.h>
#include <NetKit/NKURI.h>
#include <NetKit/NKError.h>
#include <memory>
#include <string>
#include <sstream>


#define DECLARE_PERSISTENT_OBJECT( NAME, TABLENAME )		\
typedef netkit::database::iterator< NAME, NAME > iterator;	\
static const std::string&							\
table_name()										\
{													\
	static std::string name( #TABLENAME );			\
	return name;									\
}													\
virtual const std::string&							\
table_name_v() const								\
{													\
	return table_name();							\
}													\
static NAME::ref									\
create_from_database(const netkit::database::statement::ref &stmt);\
static size_t										\
count()												\
{													\
	return netkit::database::object::count<NAME>();			\
}													\
static iterator										\
find()												\
{													\
	return netkit::database::object::find<NAME, NAME>();		\
}													\
static ref											\
find( int64_t oid )									\
{													\
	return netkit::database::object::find<NAME, NAME>( oid );	\
}													\
template <class T>									\
static iterator										\
find( const std::string &key, T val )				\
{													\
	return netkit::database::object::find<NAME, NAME>( key, val );	\
}													\
static void											\
clear()												\
{													\
	return netkit::database::object::clear<NAME>();	\
}													\
NAME( const netkit::database::statement::ref &stmt );\
static bool initialize();							\
virtual bool save( bool force = false ) const;

namespace netkit {

namespace database {

typedef std::list< std::uint64_t > oids;

struct action
{
	enum
	{
		update,
		delet
	};
};

extern std::string
sanitize( const std::string &str );


class NETKIT_DLL statement : public netkit::object
{
public:

	typedef smart_ref< statement > ref;

	virtual bool
	step() = 0;

	virtual bool
	bool_at_column( int col ) const = 0;

	virtual int
	int_at_column( int col ) const = 0;

	virtual int64_t
	int64_at_column( int col ) const = 0;

	virtual std::string
	text_at_column( int col ) const = 0;
};


template <class BaseType, class ConcreteType>
class NETKIT_DLL iterator
{
public:

	typedef typename ConcreteType::ref Ref;

	iterator()
	{
	}

	iterator( const statement::ref &stmt )
	:
		m_stmt( stmt )
	{
		operator++();
	}

	iterator( const iterator &that )
	:
		m_object( that.m_object ),
		m_stmt( that.m_stmt )
	{
	}
	
	~iterator()
	{
	}
	
	iterator&
	operator=( const statement::ref &stmt )
	{
		m_stmt = stmt;
		
		operator++();
		
		return *this;
	}

	Ref
	operator++()
	{
		if ( m_stmt->step() )
		{
			m_object = netkit::dynamic_pointer_cast< ConcreteType, BaseType >( BaseType::create_from_database( m_stmt ) );
		}
		else
		{
			m_object = nullptr;
		}
		
		return m_object;
	}

	Ref
	operator++( int i )
	{
		return operator++();
	}

	Ref
	operator*() const
	{
		return m_object;
	}

private:

	Ref				m_object;
	statement::ref	m_stmt;
};


class NETKIT_DLL manager : public object {
public:

	typedef std::function < std::string ( void ) >							backup_hook_f;
	typedef std::function < std::string ( void ) >							restore_hook_f;
	typedef std::function< void ( std::int32_t action, const oids &oids ) > observer_reply_f;
	typedef smart_ref< manager >											ref;
	
	static bool
#if defined( UNICODE )
	create( const std::wstring &s );
#else
	create( const std::string &s );
#endif
	
	static manager::ref
	instance();
	
	virtual bool
	is_connected() const = 0;

	virtual void
	on_change( netkit::cookie::ref *cookie, const std::string &table_name, observer_reply_f reply ) = 0;
	
	virtual void
	set_ignore_changes( bool val ) = 0;

	virtual netkit::status
	exec( const std::string &str, bool quiet = false ) = 0;

	virtual statement::ref
	select( const std::string &str ) = 0;
	
	virtual int64_t
	last_row_id() = 0;

	virtual std::uint32_t
	version() const = 0;

	virtual void
	set_version( std::uint32_t version ) = 0;

	virtual bool
	backup( const std::string &db, backup_hook_f hook ) = 0;

	virtual bool
	restore( const std::string &db, backup_hook_f hook ) = 0;

	virtual bool
	close() = 0;
};


class NETKIT_DLL object : public netkit::object
{
public:

	typedef smart_ref< object > ref;
	typedef std::uint64_t		oid_t;

	object( std::uint64_t oid = 0 )
	:
		m_dirty( false ),
		m_oid( oid )
	{
	}

	object( const object &that )
	:
		netkit::object( that ),
		m_oid( that.m_oid ),
		m_dirty( that.m_dirty )
	{
	}

	object( const json::value::ref &root )
	:
		netkit::object( root ),
		m_dirty( false ),
		m_oid( 0 )
	{
		inflate( root );
	}

	object( const database::statement::ref &stmt )
	:
		m_oid( stmt->int64_at_column( 0 ) ),
		m_dirty( false )
	{
	}
	
	virtual ~object()
	{
	}

	virtual void
	flatten( json::value::ref &root ) const
	{
		netkit::object::flatten( root );
		root[ "oid" ] = m_oid;
	}

	void
	inflate( const json::value::ref &root )
	{
		m_oid = root[ "oid" ]->as_uint64();
	}

	inline bool
	operator==( const object &obj )
	{
		return ( m_oid == obj.m_oid );
	}
	
	template <class Type>
	static size_t
	count()
	{
		std::ostringstream	os;
		statement::ref		stmt;
		int					rows = 0;
		
		os << "SELECT COUNT(*) FROM " << Type::table_name() << ";";
		
		stmt = manager::instance()->select( os.str() );
		
		if ( stmt->step() )
		{
			rows = stmt->int_at_column( 0 );
		}
		
		return ( size_t ) rows;
	}
	
	template <class BaseType, class ConcreteType>
	static iterator< BaseType, ConcreteType >
	find()
	{
		std::ostringstream os;

		os << "SELECT * FROM " << BaseType::table_name() << ";";

		statement::ref s = manager::instance()->select( os.str() );

		return iterator<BaseType, ConcreteType>( s );
	}

	template <class BaseType, class ConcreteType>
	static typename ConcreteType::ref
	find( int64_t oid )
	{
		std::ostringstream			os;
		statement::ref				stmt;
		typename ConcreteType::ref	t;
		
		os << "SELECT * FROM " << BaseType::table_name() << " WHERE oid = " << oid << ";";
		
		stmt = manager::instance()->select( os.str() );
		
		if ( stmt->step() )
		{
			t = netkit::dynamic_pointer_cast< ConcreteType, BaseType>( BaseType::create_from_database( stmt ) );
		}
		
		return t;
	}

	template <class BaseType, class ConcreteType>
	static iterator< BaseType, ConcreteType >
	find( const std::string &key, bool val )
	{
		std::ostringstream os;

		os << "SELECT * FROM " << BaseType::table_name() << " WHERE " << key << "=" << val << ";";

		statement::ref s = manager::instance()->select( os.str() );

		return iterator<BaseType, ConcreteType>( s );
	}

	template <class BaseType, class ConcreteType>
	static iterator< BaseType, ConcreteType >
	find( const std::string &key, std::uint64_t val )
	{
		std::ostringstream os;

		os << "SELECT * FROM " << BaseType::table_name() << " WHERE " << key << "=" << val << ";";

		statement::ref s = manager::instance()->select( os.str() );

		return iterator<BaseType, ConcreteType>( s );
	}

	template <class BaseType, class ConcreteType>
	static iterator< BaseType, ConcreteType >
	find( const std::string &key, const char *val )
	{
		std::ostringstream os;

		os << "SELECT * FROM " << BaseType::table_name() << " WHERE " << key << " LIKE '" << sanitize( val ) << "';";

		statement::ref s = manager::instance()->select( os.str() );

		return iterator<BaseType, ConcreteType>( s );
	}

	template <class BaseType, class ConcreteType>
	static iterator< BaseType, ConcreteType >
	find( const std::string &key, const std::string &val )
	{
		std::ostringstream os;

		os << "SELECT * FROM " << BaseType::table_name() << " WHERE " << key << " LIKE '" << sanitize( val ) << "';";

		statement::ref s = manager::instance()->select( os.str() );

		return iterator<BaseType, ConcreteType>( s );
	}
	
	virtual const std::string&
	table_name_v() const = 0;
	
	virtual bool save( bool force = false ) const = 0;
	
	inline void
	remove() const
	{
		if ( m_oid )
		{
			std::ostringstream os;
		
			os << "DELETE FROM " << table_name_v() << " WHERE oid = " << m_oid << ";";
		
			manager::instance()->exec( os.str() );
		}
	}

	template <class Type>
	inline static void
	clear()
	{
		std::ostringstream os;
		
		os << "DELETE FROM " << Type::table_name() << ";";
		
		manager::instance()->exec( os.str() );
	}

	inline oid_t
	oid() const
	{
		return m_oid;
	}

	bool
	dirty() const
	{
		return m_dirty;
	}

protected:

	mutable oid_t	m_oid;
	mutable bool	m_dirty;
};

}

}

#endif
