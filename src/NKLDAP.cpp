#include <NetKit/NKLDAP.h>
#include <NetKit/NKJSON.h>

using namespace netkit;


ldap::domain::domain()
{
}


ldap::domain::domain( const std::string &name, const std::string &username, const std::string &password )
:
	m_name( name ),
	m_admin_username( username ),
	m_admin_password( password )
{
}


ldap::domain::domain( const json::value::ref &root )
:
	netkit::object( root )
{
	inflate( root );
}


ldap::domain::~domain()
{
}


bool
ldap::domain::equals( const object &that ) const
{
	const ldap::domain *nthat = dynamic_cast< const ldap::domain* >( &that );
	bool ok = false;

	if ( nthat )
	{
		ok = ( m_name == nthat->m_name );
	}

	return ok;
}


void
ldap::domain::inflate( const json::value::ref &root )
{
	m_name				= root[ "name" ]->as_string();
	m_admin_username	= root[ "admin-username" ]->as_string();
	m_admin_password	= root[ "admin-password" ]->as_string();
}


void
ldap::domain::flatten( json::value::ref &root ) const
{
	netkit::object::flatten( root );

	root[ "name" ]				= m_name;
	root[ "admin-username" ]	= m_admin_username;
	root[ "admin-password" ]	= m_admin_password;
}
