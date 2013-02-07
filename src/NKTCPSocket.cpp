#include <CoreApp/CATCPSocket.h>
#include <CoreApp/CAServer.h>
#include <CoreApp/CALog.h>
#include <CoreApp/CAOS.h>
#include <CoreApp/CAError.h>
#include <sys/socket.h>
#include <openssl/pem.h>
#include <openssl/conf.h>
#include <openssl/x509v3.h>
#ifndef OPENSSL_NO_ENGINE
#	include <openssl/engine.h>
#endif

using namespace coreapp::tcp;

X509		*client::m_cert			= NULL;
EVP_PKEY	*client::m_pkey			= NULL;
BIO_METHOD	client::m_tls_methods	=
{
	BIO_TYPE_SOCKET,
	"http",
	client::tls_write,
	client::tls_read,
	client::tls_puts,
	NULL, /* http_bio_gets, */
	client::tls_ctrl,
	client::tls_new,
	client::tls_free,
	NULL,
};


client::client()
:
	socket( AF_INET, SOCK_STREAM ),
	m_connected( false ),
	m_tls_context( NULL ),
	m_tls( NULL )
{
}


client::client( socket::native fd )
:
	socket( fd ),
	m_connected( true ),
	m_tls_context( NULL ),
	m_tls( NULL )
{
}


client::~client()
{
	if ( m_tls_context )
	{
		SSL_CTX_free( m_tls_context );
		m_tls_context = NULL;
	}

	if ( m_tls )
	{
		SSL_free( m_tls );
		m_tls = NULL;
	}
}


void
client::connect( ip::address::ptr address, connect_reply reply )
{
	dispatch_async( dispatch_get_global_queue( DISPATCH_QUEUE_PRIORITY_DEFAULT, 0 ), ^()
	{
		sockaddr_storage	saddr	= address->sockaddr();
		socklen_t			slen	= saddr.ss_len;
		int					ret;
		
		if ( m_recv_source )
		{
			dispatch_release( m_recv_source );
			m_recv_source = NULL;
		}
		
		set_blocking( true );
		
		ret = ::connect( m_fd, ( struct sockaddr* ) &saddr, slen );
		
		set_blocking( false );
		
		if ( ret == -1 )
		{
			calog( log::error, "connect errno = %d", errno );
		}
		
		dispatch_async( dispatch_get_main_queue(), ^()
		{
			set_recv_handler( m_recv_handler );
			reply( ret );
		} );
	} );
}


ssize_t
client::send( const uint8_t *buf, size_t len ) const
{
	ssize_t total = 0;

	while ( len )
	{
		ssize_t num;

		if ( m_tls )
		{
			num = SSL_write( m_tls, buf + total, ( int ) len );
		}
		else
		{
			num = ::send( m_fd, buf + total, len, 0 );
		}

		if ( num > 0 )
		{
			len -= num;
			total += num;
		}
		else if ( num == 0 )
		{
			break;
		}
		else if ( num < 0 )
		{
			if ( os::error() == socket::error::wouldblock )
			{
				fd_set fds;

				FD_ZERO( &fds );

				FD_SET( m_fd, &fds );

				if ( select( m_fd + 1, NULL, &fds, NULL, NULL ) < 0 )
				{
					total = -1;
					break;
				}
			}
			else
			{
				total = ( total > 0 ) ? total : -1;
				break;
			}
		}
	}

	return total;
}


ssize_t
client::recv( uint8_t *buf, size_t len ) const
{
	ssize_t num;
	
	if ( m_tls )
	{
		num = SSL_read( m_tls, buf, ( int ) len );
	}
	else
	{
		num = ::recv( m_fd, buf, len, 0 );
	}
	
	return num;
}

	
ssize_t
client::peek( uint8_t *buf, size_t len ) const
{
	return ::recv( m_fd, buf, len, MSG_PEEK );
}


int
client::tls_setup()
{
    BIO *bio_err	= NULL;
	bool ok;
	int ret = okay;

	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();
	SSL_library_init();

    CRYPTO_mem_ctrl( CRYPTO_MEM_CHECK_ON );

    bio_err = BIO_new_fp( stderr, BIO_NOCLOSE );
    
    if ( !bio_err )
    {
		goto exit;
	}

    ok = mkcert( &m_cert, &m_pkey, 512, 0, 3650 );
    
    if ( !ok )
    {
		ret = -1;
		goto exit;
	}

exit:

#ifndef OPENSSL_NO_ENGINE
    ENGINE_cleanup();
#endif
    CRYPTO_cleanup_all_ex_data();

	if ( bio_err )
	{
		CRYPTO_mem_leaks( bio_err );
		BIO_free( bio_err );
	}
    
    return ret;
}


int
client::tls_connect()
{
	BIO				*bio;           /* BIO data */
	unsigned long	error;          /* Error code */
	int				ret = okay;
	
	if ( !m_cert )
	{
		ret = tls_setup();
		
		if ( ret != okay )
		{
			goto exit;
		}
	}
	
	m_tls_context = SSL_CTX_new( SSLv23_client_method() );
	
	if ( !m_tls_context )
	{
		ERR_print_errors_fp( stderr );
		ret = -1;
		goto exit;
	}
		
	SSL_CTX_set_options( m_tls_context, SSL_OP_NO_SSLv2 );
		
	bio = BIO_new( &m_tls_methods );
	BIO_ctrl( bio, BIO_C_SET_FILE_PTR, 0, ( char* ) this );
		
	m_tls = SSL_new( m_tls_context );
	SSL_set_bio( m_tls, bio, bio );
	
	if ( SSL_connect( m_tls ) != 1 )
	{
		while ( ( error = ERR_get_error() ) != 0 );
		SSL_CTX_free( m_tls_context );
		m_tls_context = NULL;
		SSL_free( m_tls );
		m_tls = NULL;
		ret = -1;
		goto exit;
	}

exit:

	return ret;
}


int
client::tls_accept()
{
	BIO				*bio;           /* BIO data */
	unsigned long	error;          /* Error code */
	int				ret = okay;
	
	if ( !m_cert )
	{
		ret = tls_setup();
		
		if ( ret != okay )
		{
			goto exit;
		}
	}
	
	m_tls_context = SSL_CTX_new( SSLv23_server_method() );
	
	if ( !m_tls_context )
	{
		ERR_print_errors_fp( stderr );
		ret = -1;
		goto exit;
	}
		
	SSL_CTX_set_options( m_tls_context, SSL_OP_NO_SSLv2 );
	
	SSL_CTX_use_PrivateKey( m_tls_context, m_pkey );
	SSL_CTX_use_certificate( m_tls_context, m_cert );
		
	bio = BIO_new( &m_tls_methods );
	BIO_ctrl( bio, BIO_C_SET_FILE_PTR, 0, ( char* ) this );
		
	m_tls = SSL_new( m_tls_context );
	SSL_set_bio( m_tls, bio, bio );
	
	if ( SSL_accept( m_tls ) != 1 )
	{
		while ( ( error = ERR_get_error() ) != 0 );
		SSL_CTX_free( m_tls_context );
		m_tls_context = NULL;
		SSL_free( m_tls );
		m_tls = NULL;
		ret = -1;
		goto exit;
	}

exit:

	return ret;
}


int
client::open()
{
	if ( m_fd == null )
	{
		m_fd = ::socket( AF_INET, SOCK_STREAM, 0 );
	}
	
	return ( m_fd != null ) ? 0 : -1;
}


void
client::close()
{
	if ( m_connected )
	{
		::shutdown( m_fd, SHUT_RDWR );
		m_connected = false;
	}
	
	socket::close();
}


int
client::tls_new( BIO *h )
{
	if ( !h )
	{
		return (0);
	}

	h->init  = 0;
	h->num   = 0;
	h->ptr   = NULL;
	h->flags = 0;

	return (1);
}


int
client::tls_free( BIO *h )
{
	if ( !h )
	{
		return (0);
	}

	if ( h->shutdown )
	{
		h->init  = 0;
		h->flags = 0;
	}

	return (1);
}


long
client::tls_ctrl(BIO *h, int cmd, long arg1, void *arg2 )
{
	switch ( cmd )
	{
		case BIO_CTRL_RESET:
		{
			h->ptr = NULL;
			return (0);
		}
		break;

		case BIO_C_SET_FILE_PTR:
		{
			h->ptr  = arg2;
			h->init = 1;
			return (1);
		}
		break;

		case BIO_C_GET_FILE_PTR:
		{
			if ( arg2 )
			{
				*( ( void ** ) arg2 ) = h->ptr;
				return (1);
			}
			else
			{
				return (0);
			}
		}
		break;

		case BIO_CTRL_DUP:
		case BIO_CTRL_FLUSH:
		{
			return (1);
		}
		break;
		
		default:
		{
			return (0);
		}
		break;
	}
}


int
client::tls_puts( BIO *h, const char *str )
{
	client *self = reinterpret_cast< client* >( h->ptr );
	
	return ( int ) ( ::send( self->m_fd, ( const buf_t ) str, ( int ) strlen( str ), 0 ) );
}


int
client::tls_read( BIO *h, char *buf, int size )
{
	client	*self = reinterpret_cast< client* >( h->ptr );
	int				ret;

	ret = ( int ) ::recv( self->m_fd, ( buf_t ) buf, size, 0 );

	return ret;
}


int
client::tls_write(BIO *h, const char *buf, int num )
{
	client	*self = reinterpret_cast< client* >( h->ptr );
	int				ret;

	ret = ( int ) ::send( self->m_fd, ( const buf_t ) buf, num, 0 );

	return ret;
}

		
bool
client::mkcert(X509 **x509p, EVP_PKEY **pkeyp, int bits, int serial, int days)
{
	X509 *x;
	EVP_PKEY *pk;
	RSA *rsa;
	X509_NAME *name=NULL;
	
	if ((pkeyp == NULL) || (*pkeyp == NULL))
	{
		if ((pk=EVP_PKEY_new()) == NULL)
		{
			return(false);
		}
	}
	else
	{
		pk= *pkeyp;
	}

	if ((x509p == NULL) || (*x509p == NULL))
	{
		if ((x=X509_new()) == NULL)
			goto err;
	}
	else
			x= *x509p;

	rsa=RSA_generate_key(bits,RSA_F4,callback,NULL);
	if (!EVP_PKEY_assign_RSA(pk,rsa))
	{
		goto err;
	}
	rsa=NULL;

	X509_set_version(x,2);
	ASN1_INTEGER_set(X509_get_serialNumber(x),serial);
	X509_gmtime_adj(X509_get_notBefore(x),0);
	X509_gmtime_adj(X509_get_notAfter(x),(long)60*60*24*days);
	X509_set_pubkey(x,pk);

	name=X509_get_subject_name(x);

	/* This function creates and adds the entry, working out the
	 * correct string type and performing checks on its length.
	 * Normally we'd check the return value for errors...
	 */
	X509_NAME_add_entry_by_txt(name,"C", MBSTRING_ASC, ( const unsigned char* ) "US", -1, -1, 0);
	X509_NAME_add_entry_by_txt(name,"CN", MBSTRING_ASC, ( const unsigned char* ) "OpenSSL Group", -1, -1, 0);

	/* Its self signed so set the issuer name to be the same as the
	 * subject.
	 */
	X509_set_issuer_name(x,name);

	/* Add various extensions: standard extensions */
	add(x, NID_basic_constraints, "critical,CA:TRUE");
	add(x, NID_key_usage, "critical,keyCertSign,cRLSign");

	add(x, NID_subject_key_identifier, "hash");

	/* Some Netscape specific extensions */
	add(x, NID_netscape_cert_type, "sslCA");

	add(x, NID_netscape_comment, "example comment extension");


#ifdef CUSTOM_EXT
	/* Maybe even add our own extension based on existing */
	{
			int nid;
			nid = OBJ_create("1.2.3.4", "MyAlias", "My Test Alias Extension");
			X509V3_EXT_add_alias(nid, NID_netscape_comment);
			add_ext(x, nid, "example comment alias");
	}
#endif
	
	if (!X509_sign(x,pk,EVP_md5()))
			goto err;

	*x509p=x;
	*pkeyp=pk;
	return ( true );
err:
	return( false );
}


int
client::add(X509 *cert, int nid, const char *value)
{
	X509_EXTENSION *ex;
	X509V3_CTX ctx;
	/* This sets the 'context' of the extensions. */
	/* No configuration database */
	X509V3_set_ctx_nodb(&ctx);
	/* Issuer and subject certs: both the target since it is self signed,
	 * no request and no CRL
	 */
	X509V3_set_ctx(&ctx, cert, cert, NULL, NULL, 0);
	ex = X509V3_EXT_conf_nid(NULL, &ctx, nid, ( char* ) value);
	if (!ex)
			return 0;

	X509_add_ext(cert,ex,-1);
	X509_EXTENSION_free(ex);
	return 1;
}


void
client::callback(int p, int n, void *arg)
{
	char c='B';

	if (p == 0) c='.';
	if (p == 1) c='+';
	if (p == 2) c='*';
	if (p == 3) c='\n';
	
	fputc(c,stderr);
}

#if defined( __APPLE__ )
#	pragma mark server implementation
#endif

server::server( const ip::address::ptr &addr )
:
	socket( AF_INET, SOCK_STREAM ),
	m_addr( addr )
{
	listen();
}


server::~server()
{
}


int
server::open()
{
	if ( m_fd == null )
	{
		m_fd = ::socket( AF_INET, SOCK_STREAM, 0 );
	}
	
	return ( m_fd != null ) ? 0 : -1;
}


void
server::add_listener( coreapp::server::ptr listener )
{
	m_listeners.push_back( listener );
}


int
server::listen()
{
	sockaddr_storage	saddr	= m_addr->sockaddr();
	sockaddr_storage	saddr2;
	socklen_t			slen;
	int					ret;
	
	ret = ::bind( m_fd, ( struct sockaddr* ) &saddr, saddr.ss_len );
	
	if ( ret != 0 )
	{
		calog( log::error, "bind() failed: %d", os::error() );
		goto exit;
	}
	
	slen = sizeof( saddr2 );
	ret = ::getsockname( m_fd, ( struct sockaddr* ) &saddr2, &slen );
	
	if ( ret != 0 )
	{
		calog( log::error, "getsockname() failed: %d", os::error() );
		goto exit;
	}
	
	if ( saddr2.ss_family == AF_INET )
	{
		m_addr->set_port( ntohs( ( ( struct sockaddr_in* ) &saddr2 )->sin_port ) );
	}
	else if ( saddr.ss_family == AF_INET6 )
	{
		m_addr->set_port( ntohs( ( ( struct sockaddr_in6* ) &saddr2 )->sin6_port ) );
	}
	
	ret = ::listen( m_fd, 5 );
	
	if ( ret != 0 )
	{
		calog( log::error, "listen() failed: %d", os::error() );
		goto exit;
	}
	
	set_recv_handler( [ this ]()
	{
		client::ptr			sock;
		ip::address::ptr	addr;
		
		sock = accept( addr );
		
		if ( sock )
		{
			sock->set_recv_handler( [ this, sock ]()
			{
				uint8_t buf[ 64 ];
				ssize_t	num;
				
				num = sock->peek( buf, sizeof( buf ) );
				
				if ( num > 0 )
				{
					for ( listeners::iterator it = m_listeners.begin(); it != m_listeners.end(); it++ )
					{
						if ( ( *it )->adopt( sock, buf, num ) )
						{
							break;
						}
					}
				}
			} );
		}
	} );
	
exit:

	return ret;
}


client::ptr
server::accept( ip::address::ptr &addr )
{
	native					newFd;
	struct sockaddr_storage	peer;
	socklen_t				peerLen = sizeof( peer );
	client::ptr				newSock;
	
	newFd = ::accept( m_fd, ( struct sockaddr* ) &peer, &peerLen );
	
	if ( newFd != socket::null )
	{
		try
		{
			char buf[ 256 ];
#if defined( WIN32 )
			DWORD bufSize = sizeof( buf );
#endif

			addr = new ip::address( peer );

			newSock = new client( newFd );
			//memcpy( &newSock->m_peer, &peer, sizeof( peer ) );
			//newSock->m_peerLen	= peerLen;
			
			if ( peer.ss_family == AF_INET )
			{
#if defined( WIN32 )
				struct sockaddr_storage dummy;

				memcpy( &dummy, &peer, sizeof( peer ) );

				( ( struct sockaddr_in* ) &dummy )->sin_port = 0;

				if ( WSAAddressToStringA( ( LPSOCKADDR ) &dummy, sizeof( dummy ), NULL, buf, &bufSize ) == 0 )
#else
				if ( inet_ntop( peer.ss_family, &( ( ( struct sockaddr_in *) &peer )->sin_addr ), buf, sizeof( buf ) ) != NULL )
#endif
				{
					//newSock->m_peerHost			= buf;
					//newSock->getEthernetAddr();
				}
			}
			else
			{
#if defined( WIN32 )
				struct sockaddr_storage dummy;

				memcpy( &dummy, &peer, sizeof( peer ) );

				( ( struct sockaddr_in6* ) &dummy )->sin6_port = 0;

				if ( WSAAddressToStringA( ( LPSOCKADDR ) &dummy, sizeof( dummy ), NULL, buf, &bufSize ) == 0 )
#else
				if ( inet_ntop( peer.ss_family, &( ( ( struct sockaddr_in6* ) &peer )->sin6_addr ), buf, sizeof( buf ) ) != NULL )
#endif
				{
					//newSock->m_peerHost			= buf;
					//newSock->m_peerEthernetAddr = m_peerHost;
				}
			}
		}
		catch ( ... )
		{
		}
	}
	
	return newSock;
}

