#ifndef _netkit_proxy_h
#define _netkit_proxy_h

#include <NetKit/NKURI.h>
#include <string>

namespace netkit {

class proxy : public object
{
public:

	typedef smart_ptr< proxy > ptr;

	proxy( uri::ptr uri );

	virtual ~proxy();

	inline const uri::ptr&
	uri() const
	{
		return m_uri;
	}
	
	inline void
	set_uri( const uri::ptr &uri )
	{
		m_uri = uri;
	}

	inline const std::string&
	host() const
	{
		return m_host;
	}

	inline void
	set_host( const std::string &host )
	{
		m_host = host;
	}

	inline const std::string&
	bypass() const
	{
		return m_bypass;
	}

	inline void
	set_bypass( const std::string &bypass )
	{
		m_bypass = bypass;
	}

	inline std::uint16_t
	port() const
	{
		return m_port;
	}

	inline void
	set_port( std::uint16_t port )
	{
		m_port = port;
	}

	inline const std::string&
	user() const
	{
		return m_user;
	}

	inline void
	set_user( const std::string &user )
	{
		m_user = user;
	}

	inline const std::string&
	password() const
	{
		return m_password;
	}

	inline void
	set_password( const std::string &password )
	{
		m_password = password;
	}
	
private:

	uri::ptr		m_uri;
	std::string		m_host;
	std::string		m_bypass;
	std::uint16_t	m_port;
	std::string		m_user;
	std::string		m_password;
};

}

#endif