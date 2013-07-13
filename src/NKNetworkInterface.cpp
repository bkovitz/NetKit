#include <NetKit/NKNetworkInterface.h>
#include <NetKit/NKUnicode.h>
#include <NetKit/NKJSON.h>
#include <NetKit/NKMacros.h>

using namespace netkit;

nic::nic()
:
	m_flags( 0 )
{
}


nic::nic( const json::value::ref &root )
:
	netkit::object( root )
{
	inflate( root );
}


bool
nic::equals( const object &that ) const
{
	const nic *nthat = dynamic_cast< const nic* >( &that );
	bool ok = false;

	if ( nthat )
	{
		ok = ( m_name == nthat->m_name );
	}

	return ok;
}


void
nic::inflate( const json::value::ref &root )
{
	m_name			= root[ "name" ]->as_string();
	m_display_name	= root[ "display-name" ]->as_string();
	m_dns_suffix	= root[ "dns-suffix" ]->as_string();
	m_flags			= root[ "flags" ]->as_int32();

	for ( auto i = 0; i < root[ "addresses" ]->size(); i++ )
	{
		m_addresses.push_back( new ip::address( root[ "addresses" ][ i ] ) );
	}
}


void
nic::flatten( json::value::ref &root ) const
{
	netkit::object::flatten( root );

	root[ "name" ]			= m_name;
	root[ "display-name" ]	= m_display_name;
	root[ "dns-suffix" ]	= m_dns_suffix;
	root[ "flags" ]			= m_flags;

	for ( auto it = m_addresses.begin(); it != m_addresses.end(); it++ )
	{
		root[ "address" ]->append( ( *it )->json() );
	}
}