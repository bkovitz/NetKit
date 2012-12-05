#ifndef _CoreApp_uri_h
#define _CoreApp_uri_h

#include <CoreApp/object.h>
#include <CoreApp/smart_ptr.h>
#include <CoreApp/tstring.h>

namespace CoreApp {

class uri : public object
{
public:

	typedef smart_ptr< uri > ptr;
	
	uri();
	
	uri( const std::tstring& s );

	~uri();

	inline const std::tstring&
	scheme() const
	{
		return m_scheme;
	}
	
	inline void
	set_scheme( const std::tstring &val )
	{
		m_scheme = val;
	}

	inline const std::tstring&
	host() const
	{
		return m_host;
	}
	
	inline void
	set_host( const std::tstring &val )
	{
		m_host = val;
	}

	inline int
	port() const
	{
		return m_port;
	}

	inline void
	set_port( int val )
	{
		m_port = val;
	}

	inline const std::tstring&
	path() const
	{
		return m_path;
	}

	inline void
	set_path( const std::tstring &val )
	{
		m_path = val;
	}

	inline const std::tstring&
	query() const
	{
		return m_query;
	}
	
	inline void
	set_query( const std::tstring &val )
	{
		m_query = val;
	}
	
	std::tstring
	recompose();

	void
	assign( const std::tstring &s );
	
	void
	clear();
	
	static std::tstring
	encode( const std::tstring &str );
	
	static std::tstring
	decode( const std::tstring& str );

private:

	std::tstring	m_scheme;
	std::tstring	m_host;
	int				m_port;
	std::tstring	m_path;
	std::tstring	m_query;
};

}

#endif
