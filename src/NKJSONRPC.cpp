#include <NetKit/NKJSONRPC.h>
#include <NetKit/NKRunLoop.h>
#include <NetKit/NKPlatform.h>

using namespace netkit::jsonrpc;

connection::connection()
:
	m_base( NULL ),
	m_eptr( NULL ),
	m_end( NULL ),
	m_id( 1 )
{
	add( 4192 );
}


connection::connection( const tcp::client::ptr &sock )
:
	super( sock ),
	m_base( NULL ),
	m_eptr( NULL ),
	m_end( NULL ),
	m_id( 1 )
{
//	runloop::instance()->registerSocket( this, runloop::event::read, dataNotification, this );
//	m_connections.push_back( this );
	add( 4192 );
}


connection::~connection()
{
	free( m_base );
}


void
connection::data_notification( socket::ptr sock, int event, void *context )
{
	connection	*self = ( connection* ) context;
	ssize_t		num;
	
	while ( 1 )
	{
		if ( self->numBytesUnused() == 0 )
		{
			self->add( self->size() );
		}
		
		num = self->recv( ( std::uint8_t* ) self->m_eptr, self->numBytesUnused(), 0 );
	
		if ( num > 0 )
		{
			self->m_eptr += num;

			if ( !self->process() )
			{
				self->shutdown();
				break;
			}
		}
		else if ( num < 0 )
		{
			if ( os::error() != os::error::wouldblock )
			{
				self->shutdown();
			}
			
			break;
		}
		else
		{
			self->shutdown();
			break;
		}
	}
}


bool
connection::send_notification( json::value &notification )
{
	return send( notification);
}


bool
connection::send_request( json::value &request, reply reply )
{
	std::int32_t id = m_id++;

	request[ "id" ]			= id;
	m_active_requests[ id ]	= reply;
	
	return send( request );
}


int
connection::send_response( json::value &response )
{
	return send( response );
}


bool
connection::send( Json::Value &value )
{
	std::string			msg;
	int					bytes_left;
	int					bytes_written;
	
	msg = value.flatten( 0 );
    msg = encode( msg );
    
	bytes_left		= ( int ) msg.size();
	bytes_written	= 0;

    while ( bytes_left )
    {
		ssize_t num = socket::send( ( const std::uint8_t* ) msg.c_str() + bytes_written, bytes_left, 0 );

		if ( num > 0 )
		{
			bytes_left		-= num;
			bytes_written	+= num;
		}
		else if ( num == 0 )
		{
			break;
		}
		else if ( fingerprint::os::error() == fingerprint::os::error::wouldblock )
		{
			fingerprint::os::catnap( 1 );
		}
		else
		{
			break;
		}
	}
	
	return ( bytes_left == 0 ) ? true : false;
}


bool
connection::process()
{
	Json::Value		root;
	Json::Value		error;
	bool			parsing = false;
    unsigned long	len = 0;
	size_t			index = 0;
	size_t			i = 0;
	std::uint8_t	*colon;
	std::string		msg;
	bool			ok = true;

    while ( num_bytes_used() )
    {
		index = -1;
	    
		for ( colon = m_base; colon != m_eptr; colon++ )
		{
			if ( *colon == ':' )
			{
				index = colon - m_base;
				break;
			}
		}
		
		if ( index == -1 )
		{
			// If we have more than 10 bytes and no ':', then let's assume this buffer is no good

			ok = num_bytes_used() < 10;
			goto exit;
		}

		len = 0;

		for ( i = 0 ; i < index ; i++ )
		{
			if ( isdigit( m_base[i] ) )
			{
        		len = len * 10 + ( m_base[ i ] - ( char ) 0x30 );
			}
			else
			{
				ok = false;
				goto exit;
			}
		}
		
		if ( size() < ( index + len + 2 ) )
		{
			add( ( index + len + 2 ) - size() );
			goto exit;
		}
		
		if ( num_bytes_used() < len )
		{
			goto exit;
		}

		if ( m_base[ index + len + 1 ] != ',' )
		{
			ok = false;
			goto exit;
		}

		msg.assign( ( const char* ) m_base, index + 1, len );
	    
		shift( index + len + 2 );

		parsing = m_reader.parse( msg, root );
	      
		if ( !parsing )
		{
			ok = false;
			goto exit;
		}

		if ( m_delegate )
		{
			if ( root[ "method" ] != json::value::null )
			{
				request_handlers::iterator it = m_request_handlers.find( root[ "method"].asString() );
				
				if ( it != m_request_handlers.end() )
				{
					it->second( root, [=]( json::value::ptr &response )
					{
						if ( response != Json::Value::null )
						{
							connection::ptr nckeep( keep );
							nckeep->send( response );
						}
					} );
				connection::delegate	*delegate = dynamic_cast< connection::delegate* >( m_delegate );
				Json::Value				response;
				
				if ( delegate )
				{
					connection::ptr keep( this );

					delegate->handleRequest( keep, root, FINGERPRINT_CLOSURE_BYCOPY( Json::Value &response )
					{
						if ( response != Json::Value::null )
						{
							connection::ptr nckeep( keep );
							nckeep->send( response );
						}
					} );
				}
			}
			else
			{
				activeRequests::iterator it = m_activeRequests.find( root[ "id" ].asInt() );
				
				if ( it != m_activeRequests.end() )
				{
					it->second( root );

					FINGERPRINT_RELEASE_CLOSURE( it->second );

					m_activeRequests.erase( it );
				}
			}
		}
	}
	
exit:

	return ok;
}


std::string
connection::getString(Json::Value value)
{
	return m_writer.write(value);
}


std::string
connection::encode( const std::string &msg )
{
	char buf[ 64 ];
	
	fingerprint::os::sprintf( buf, sizeof( buf ), sizeof( buf ), "%lu:", msg.size() );
	return std::string( buf ) + msg + ",";
}

#define INVALID_REQUEST -1
bool
connection::validate(const Json::Value& root, Json::Value& error)
{
      Json::Value err;
      
      /* check the JSON-RPC version => 2.0 */
      if(!root.isObject() || !root.isMember("jsonrpc") || root["jsonrpc"] != "2.0") 
      {
        error["id"] = Json::Value::null;
        error["jsonrpc"] = "2.0";
        
        err["code"] = INVALID_REQUEST;
        err["message"] = "Invalid JSON-RPC request.";
        error["error"] = err;
        return false;
      }

      if(root.isMember("id") && (root["id"].isArray() || root["id"].isObject()))
      {
        error["id"] = Json::Value::null;
        error["jsonrpc"] = "2.0";

        err["code"] = INVALID_REQUEST;
        err["message"] = "Invalid JSON-RPC request.";
        error["error"] = err;
        return false;
      }

      /* extract "method" attribute */
      if(!root.isMember("method") || !root["method"].isString())
      {
        error["id"] = Json::Value::null;
        error["jsonrpc"] = "2.0";

        err["code"] = INVALID_REQUEST;
        err["message"] = "Invalid JSON-RPC request.";
        error["error"] = err;
        return false;
      }

	return true;
}


bool
connection::process(const Json::Value& root, Json::Value& response)
{
	Json::Value error;
	std::string method;

	if ( !validate(root, error ) )
	{
		response = error;
		return false;
	}
#if 0
      method = root[ "method" ].asString();
      
      if(method != "")
      {
        CallbackMethod* rpc = Lookup(method);
        if(rpc)
        {
          return rpc->Call(root, response);
        }
      }
      
      /* forge an error response */
      response["id"] = root.isMember("id") ? root["id"] : Json::Value::null;
      response["jsonrpc"] = "2.0";

      error["code"] = METHOD_NOT_FOUND;
      error["message"] = "Method not found.";
      response["error"] = error;
#endif
      return false;
}


void
connection::shutdown()
{
	Json::Value result;
	Json::Value error;
		
	runloop::instance()->unregisterSocket( this );
	
	close();
	
	if ( m_closure )
	{
		socket::ptr temp( this );

		m_closure( temp );
	}
	
	error[ "code" ]		= lost_connection_error;
	result[ "error" ]	= error;

	for ( std::map< int, reply>::iterator it = m_activeRequests.begin(); it != m_activeRequests.end(); it++ )
	{
		it->second( result );
	}

	for ( list::iterator it = m_connections.begin(); it != m_connections.end(); it++ )
	{
		if ( ( *it ).operator->() == this )
		{
			m_connections.erase( it );
			break;
		}
	}
}


// --------------------------------------------
// manager implementation
// --------------------------------------------

#if defined( __APPLE__ )
#pragma mark manager
#endif

DEFINE_COMPONENT1( manager )

manager::manager()
{
}


manager::~manager()
{
}


status
manager::applicationWillInitialize()
{
	m_state = okay;
	return state();
}


status
manager::applicationDidInitialize()
{
	return state();
}

	
void
manager::applicationWillTerminate()
{
}


std::string
manager::name() const
{
	return "jsonrpc";
}

	
bool
manager::adopt( const fingerprint::socket &sock, uint8_t *bytes, size_t len, socketDidClose closure )
{
	connection::ptr connection;
	unsigned		index = 0;
	bool			ok = false;

	while ( isdigit( bytes[ index ] ) && ( index < len ) )
	{
		index++;
	}
	
	if ( index == len )
	{
		goto exit;
	}
	
	if ( bytes[ index++ ] != ':' )
	{
		goto exit;
	}
	
	if ( index == len )
	{
		goto exit;
	}
	
	if ( bytes[ index ] != '{' )
	{
		goto exit;
	}

	try
	{
		connection = new jsonrpc::connection( sock, closure );
	}
	catch ( ... )
	{
		// log this
		connection = NULL;
	}
	
	if ( !connection )
	{
		goto exit;
	}

	ok = true;

exit:

	return ok;
}

#endif