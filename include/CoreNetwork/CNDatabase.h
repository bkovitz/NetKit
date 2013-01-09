#ifndef _coreapp_database_h
#define _coreapp_database_h

#include <CoreApp/CASmartPtr.h>
#include <CoreApp/CAObject.h>
#include <CoreApp/CAURI.h>
#include <CoreApp/CATypes.h>
#include <CoreApp/CAError.h>
#include <memory>
#include <string>
#include <sstream>


#define DECLARE_PERSISTENT_OBJECT( NAME )			\
typedef coreapp::database::iterator< NAME, ptr > iterator;	\
static const std::string&							\
table_name()										\
{													\
	static std::string name( #NAME );				\
	return name;									\
}													\
virtual const std::string&							\
table_name_v() const								\
{													\
	return table_name();								\
}													\
static size_t										\
count()												\
{													\
	return coreapp::database::object::count<NAME>();			\
}													\
static iterator										\
find()												\
{													\
	return coreapp::database::object::find<NAME, ptr>();		\
}													\
static ptr											\
find( int64_t oid )									\
{													\
	return coreapp::database::object::find<NAME, ptr>( oid );	\
}													\
static ptr											\
find( const std::string &uuid )						\
{													\
	return coreapp::database::object::find<NAME, ptr>( uuid );	\
}													\
template <class T>									\
static iterator										\
find( const std::string &key, T val )				\
{													\
	return coreapp::database::object::find<NAME, ptr>( key, val );	\
}													\
NAME( const coreapp::database::statement::ptr &stmt );\
static bool initialize();							\
virtual bool save() const;


namespace coreapp {
namespace database {


extern std::string
sanitize( const std::string &str );


class observer
{
public:
	
	virtual void
	object_was_updated( const std::string &table, int64_t oid ) = 0;
	
	virtual void
	object_was_removed( const std::string &table, int64_t oid ) = 0;
};


class statement : public coreapp::object
{
public:

	typedef smart_ptr< statement > ptr;

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


template <class Type, class Ptr>
class iterator
{
public:

	iterator()
	{
	}

	iterator( const statement::ptr &stmt )
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
	operator=( const statement::ptr &stmt )
	{
		m_stmt = stmt;
		
		operator++();
		
		return *this;
	}

	Ptr
	operator++()
	{
		if ( m_stmt->step() )
		{
			m_object = new Type( m_stmt );
		}
		else
		{
			m_object = NULL;
		}
		
		return m_object;
	}

	Ptr
	operator++( int i )
	{
		return operator++();
	}

	Ptr
	operator*() const
	{
		return m_object;
	}

private:

	Ptr				m_object;
	statement::ptr	m_stmt;
};


class manager : public coreapp::object
{
public:

	typedef smart_ptr< manager > ptr;

	static bool
	initialize( const uri::ptr &uri );

	static manager::ptr
	instance();

	virtual void
	add_observer( const std::string &table_name, observer *o ) = 0;
	
	virtual void
	remove_observer( const std::string &table_name, observer *o ) = 0;

	virtual status
	exec( const std::string &str ) = 0;

	virtual statement::ptr
	select( const std::string &str ) = 0;
	
	virtual int64_t
	last_row_id() = 0;

	virtual bool
	close() = 0;
};


class object : public coreapp::object
{
public:

	typedef smart_ptr< object > ptr;
	typedef int64_t oid_t;

	object()
	:
		m_dirty( false ),
		m_oid( 0 )
	{
	}

	object( const database::statement::ptr &stmt )
	:
		m_oid( stmt->int64_at_column( 0 ) ),
		m_uuid( stmt->text_at_column( 1 ) ),
		m_dirty( false )
	{
	}
	
	virtual ~object()
	{
	}

	inline bool
	operator==( const object &obj )
	{
		return ( m_uuid == obj.m_uuid );
	}
	
	template <class Type>
	static size_t
	count()
	{
		std::ostringstream	os;
		statement::ptr		stmt;
		int					rows = 0;
		
		os << "SELECT COUNT(*) FROM " << Type::table_name() << ";";
		
		stmt = database::manager::instance()->select( os.str() );
		
		if ( stmt->step() )
		{
			rows = stmt->int_at_column( 0 );
		}
		
		return ( size_t ) rows;
	}
	
	template <class Type, class Ptr>
	static iterator< Type, Ptr >
	find()
	{
		std::ostringstream os;

		os << "SELECT * FROM " << Type::table_name() << ";";

		statement::ptr s = database::manager::instance()->select( os.str() );

		return iterator<Type, Ptr>( s );
	}

	template <class Type, class Ptr>
	static Ptr
	find( int64_t oid )
	{
		std::ostringstream	os;
		statement::ptr		stmt;
		Type			*	t;
		
		os << "SELECT * FROM " << Type::table_name() << " WHERE oid = " << oid << ";";
		
		stmt = database::manager::instance()->select( os.str() );
		
		if ( stmt->step() )
		{
			t = new Type( stmt );
		}
		else
		{
			t = NULL;
		}
		
		return t;
	}

	template <class Type, class Ptr>
	static Ptr
	find( const std::string &uuid )
	{
		std::ostringstream	os;
		statement::ptr		stmt;
		Type			*	t;
		
		os << "SELECT * FROM " << Type::table_name() << " WHERE uuid LIKE '" << sanitize( uuid ) << "';";
		
		stmt = database::manager::instance()->select( os.str() );
		
		if ( stmt->step() )
		{
			t = new Type( stmt );
		}
		else
		{
			t = NULL;
		}
		
		return t;
	}

	template <class Type, class Ptr>
	static iterator< Type, Ptr >
	find( const std::string &key, bool val )
	{
		std::ostringstream os;

		os << "SELECT * FROM " << Type::table_name() << " WHERE " << key << "=" << val << ";";

		statement::ptr s = database::manager::instance()->select( os.str() );

		return iterator<Type, Ptr>( s );
	}

	template <class Type, class Ptr>
	static iterator< Type, Ptr >
	find( const std::string &key, int val )
	{
		std::ostringstream os;

		os << "SELECT * FROM " << Type::table_name() << " WHERE " << key << "=" << val << ";";

		statement::ptr s = database::manager::instance()->select( os.str() );

		return iterator<Type, Ptr>( s );
	}

	template <class Type, class Ptr>
	static iterator< Type, Ptr >
	find( const std::string &key, const char *val )
	{
		std::ostringstream os;

		os << "SELECT * FROM " << Type::table_name() << " WHERE " << key << " LIKE '" << sanitize( val ) << "';";

		statement::ptr s = database::manager::instance()->select( os.str() );

		return iterator<Type, Ptr>( s );
	}

	template <class Type, class Ptr>
	static iterator< Type, Ptr >
	find( const std::string &key, const std::string &val )
	{
		std::ostringstream os;

		os << "SELECT * FROM " << Type::table_name() << " WHERE " << key << " LIKE '" << sanitize( val ) << "';";

		statement::ptr s = database::manager::instance()->select( os.str() );

		return iterator<Type, Ptr>( s );
	}
	
	virtual const std::string&
	table_name_v() const = 0;
	
	virtual bool save() const = 0;
	
	inline void
	remove() const
	{
		if ( m_oid )
		{
			std::ostringstream os;
		
			os << "DELETE FROM " << table_name_v() << " WHERE oid = " << m_oid << ";";
		
			database::manager::instance()->exec( os.str() );
		}
	}

	inline const std::string&
	uuid() const
	{
		return m_uuid;
	}

	inline void
	set_uuid( const std::string &uuid )
	{
		m_uuid = uuid;
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

	mutable oid_t		m_oid;
	mutable std::string	m_uuid;
	mutable bool		m_dirty;
};

}

}

#endif
