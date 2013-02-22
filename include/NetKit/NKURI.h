#ifndef _netkit_uri_h
#define _netkit_uri_h

#include <NetKit/NKObject.h>
#include <NetKit/NKSmartPtr.h>
#include <string>

namespace netkit {

class uri : public object
{
public:

	typedef smart_ptr< uri > ptr;
	
	uri();
	
	uri( const std::string& s );

	~uri();

	inline const std::string&
	scheme() const
	{
		return m_scheme;
	}
	
	inline void
	set_scheme( const std::string &val )
	{
		m_scheme = val;
	}

	inline const std::string&
	host() const
	{
		return m_host;
	}
	
	inline void
	set_host( const std::string &val )
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

	inline const std::string&
	path() const
	{
		return m_path;
	}

	inline void
	set_path( const std::string &val )
	{
		m_path = val;
	}

	inline const std::string&
	query() const
	{
		return m_query;
	}
	
	inline void
	set_query( const std::string &val )
	{
		m_query = val;
	}
	
	std::string
	recompose() const;

	void
	assign( const std::string &s );
	
	void
	clear();
	
	static std::string
	encode( const std::string &str );
	
	static std::string
	decode( const std::string& str );

private:

	std::string	m_scheme;
	std::string	m_host;
	int				m_port;
	std::string	m_path;
	std::string	m_query;
};

}

#endif
