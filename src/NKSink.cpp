#include <NetKit/NKSink.h>
#include <NetKit/NKSource.h>

using namespace netkit;

sink::sink( const source_ptr &source )
:
	m_source( source )
{
}


sink::~sink()
{
}


ssize_t
sink::send( const std::uint8_t *buf, size_t len )
{
}

	
ssize_t
sink::read( std::uint8_t *buf, size_t len )
{
}