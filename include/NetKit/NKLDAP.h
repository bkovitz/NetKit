#ifndef _netkit_ldap_h
#define _netkit_ldap_h

#include <NetKit/NKObject.h>
#include <vector>
#include <string>

namespace netkit {

namespace ldap {

class domain : public netkit::object
{
public:

	typedef smart_ref< domain > ref;
	typedef std::vector< ref >	list;

	domain();

	domain( const std::string &name, const std::string &username, const std::string &password );

	domain( const json::value_ref &root );

	virtual ~domain();

	inline const std::string&
	name() const
	{
		return m_name;
	}

	inline void
	set_name( const std::string &val )
	{
		m_name = val;
	}

	inline const std::string&
	admin_username() const
	{
		return m_admin_username;
	}

	inline void
	set_admin_username( const std::string &val )
	{
		m_admin_username = val;
	}

	inline const std::string&
	admin_password() const
	{
		return m_admin_password;
	}

	inline void
	set_admin_password( const std::string &val )
	{
		m_admin_password = val;
	}

	virtual bool
	equals( const object &that ) const;

	virtual void
	flatten( json::value_ref &root ) const;

	void
	inflate( const json::value_ref &root );

protected:

	std::string 	m_name;
	std::string		m_admin_username;
	std::string		m_admin_password;
};

}

}

#endif
