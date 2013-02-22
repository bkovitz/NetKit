#ifndef _netkit_jsonrpc_h
#define _netkit_jsonrpc_h

#if 0
#include <NetKit/NKSink.h>
#include <NetKit/NKJSON.h>
#include <list>
#include <map>

namespace netkit {

namespace jsonrpc {

class message;
typedef smart_ptr< message > message_ptr;

class parser : public netkit::sink
{
public:

	typedef std::function< void( json::value::ptr& ) >				handler;
	typedef std::function< void( json::value::ptr&, handler h ) >	request_handler;
	typedef handler													reply_handler;

	typedef smart_ptr< connection > ptr;
	typedef std::list< ptr > list;
	
public:

	connection();

	connection( const tcp::client::ptr &sock );
	
	virtual ~connection();
	
	void
	register_request_handler( const std::string &method, request_handler handler );
	
	void
	unregister_request_handler( const std::string &method );

	bool
	send( const message_ptr &message );
	
protected:

	typedef netkit::connection< tcp::client >	super;
	
	typedef std::vector< std::string >			strings;
	
	virtual void
	can_send_data();

	virtual void
	can_recv_data();

	std::string						m_uri_value;
	std::string						m_header_field;
	std::string						m_header_value;
	int									m_operation;
	time_t								m_start;
	bool								m_okay;
	blob								m_body;
	
	int							m_parse_state;

	message_ptr					m_message;

	std::string					m_expect;
	std::string					m_host;
	std::string					m_authorization;
	std::string					m_username;
	std::string					m_password;
	
protected:

	typedef std::map< std::string, request_handler >	request_handlers;
	typedef std::map< std::int32_t, reply_handler >		reply_handlers;

	inline size_t
    num_bytes_used()
    {
        return m_end - m_eptr;
    }

    inline size_t
    size()
    {
        return m_end - m_base;
    }

    inline void
    add( size_t numBytes )
    {
        size_t bytesUsed = num_bytes_used();
        size_t oldSize = size();
        size_t newSize = oldSize + numBytes;

        m_base = ( unsigned char* ) realloc( m_base, newSize );

        m_eptr = m_base + bytesUsed;
        m_end = m_base + newSize;
    }

    inline void
    shift( size_t index )
    {
        if ( ( m_base + index ) < m_eptr )
        {
            int delta = ( int ) ( ( m_eptr - ( m_base + index ) ) );
            //memmove_s( m_base, size(), m_base + index, delta );
//            fingerprint::os::memmove( m_base, size(), m_base + index, delta );
            m_eptr = m_base + delta;
        }
        else
        {
            m_eptr = m_base;
        }
    }
	
	std::string             m_token;
	request_handlers		m_request_handlers;
	reply_handlers			m_reply_handlers;
	std::uint8_t			*m_base;
	std::uint8_t			*m_eptr;
	std::uint8_t			*m_end;
	std::int32_t			m_id;
	
private:

	bool
	build_message();
	
	void
	init( int type );
};


class service : public netkit::service
{
protected:

	virtual bool
	adopt( const socket::ptr &sock, uint8_t *peek, size_t len );
};


class client : public connection
{
};

}

}

#endif
#endif