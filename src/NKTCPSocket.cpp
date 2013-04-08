/*
 * Copyright (c) 2013, Porchdog Software Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of the FreeBSD Project.
 *
 */

#include <NetKit/NKTCPSocket.h>
#include <NetKit/NKRunLoop.h>
#include <NetKit/NKLog.h>
#include <NetKit/NKPlatform.h>
#include <NetKit/NKError.h>
#include <sys/socket.h>
#include <openssl/pem.h>
#include <openssl/conf.h>
#include <openssl/x509v3.h>
#ifndef OPENSSL_NO_ENGINE
#	include <openssl/engine.h>
#endif
#include <thread>

using namespace netkit::tcp;

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
	socket::client( AF_INET, SOCK_STREAM ),
	m_connected( false ),
	m_tls_context( NULL ),
	m_tls( NULL )
{
}


client::client( socket::native fd )
:
	socket::client( fd ),
	m_connected( true ),
	m_tls_context( NULL ),
	m_tls( NULL )
{
}


client::client( socket::native fd, const ip::address::ptr &addr )
:
	socket::client( fd ),
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
client::connect( ip::address::ptr address, connect_reply_f reply )
{
	sockaddr_storage	saddr	= address->sockaddr();
	socklen_t			slen	= saddr.ss_len;
	
	std::thread t( [=]()
	{
		int ret;
		
		set_blocking( true );
		
		ret = ::connect( m_fd, ( struct sockaddr* ) &saddr, slen );
		
		set_blocking( false );
		
		if ( ret == -1 )
		{
			nklog( log::error, "connect errno = %d", errno );
		}
		
		runloop::instance()->dispatch_on_main_thread( [=]()
		{
			reply( ret );
			
			if ( m_sink )
			{
				auto source = runloop::instance()->create_source( m_fd, runloop::event::read, [=]( runloop::source s, runloop::event e )
				{
					m_sink->process();
				} );
				
				runloop::instance()->schedule( source );
					
				set_source( source );
			}
		} );
	} );
	
	t.detach();
}


int
client::connect_sync( ip::address::ptr address )
{
	sockaddr_storage	saddr	= address->sockaddr();
	socklen_t			slen	= saddr.ss_len;
	int					ret;
		
	set_blocking( true );
	
	ret = ::connect( m_fd, ( struct sockaddr* ) &saddr, slen );
		
	set_blocking( false );
	
	if ( ret == -1 )
	{
		nklog( log::error, "connect errno = %d", errno );
	}
	
	return ret;
}


bool
client::is_secure()
{
	return ( m_tls != NULL ) ? true : false;
}

	
bool
client::secure()
{
	bool ok = true;

    // Socket is in non-blocking mode.  Set it back to blocking before we setup TLS
    // On Windows, we need to unregister the socket from the runloop first

	if ( m_source )
	{
		runloop::instance()->suspend( m_source );
	}

    // Now set it to blocking

	if ( !set_blocking( true ) )
	{
        nklog( log::error, "unable to set socket to blocking" );
        ok = false;
        goto exit;
    }

    // Now start up tls

    if ( tls_accept() != 0 )
    {
        nklog( log::error, "unable to start TLS" );
        ok = false;
        goto exit;
    }
	
	set_blocking( false );

    // And register it back with the runloop

	if ( m_source )
	{
		runloop::instance()->schedule( m_source );
	}

exit:

    return ok;

}


ssize_t
client::peek( uint8_t *buf, size_t len )
{
	return ::recv( m_fd, buf, len, MSG_PEEK );
}


ssize_t
client::recv( uint8_t *buf, size_t len )
{
	ssize_t num;
	
	if ( m_tls )
	{
		num = SSL_read( m_tls, buf, ( int ) len );
	}
	else
	{
		num = ::recv( m_fd, buf, len, 0 );
		
		if ( num == 0 )
		{
			platform::set_error( ( int ) socket::error::reset );
			num = -1;
		}
		else if ( num == -1 )
		{
			if ( platform::error() == ( int ) socket::error::would_block )
			{
				num = 0;
			}
			else
			{
				nklog( log::error, "recv() returned %d", platform::error() );
			}
		}
	}
	
	return num;
}


ssize_t
client::send( const uint8_t *buf, size_t len )
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
			if ( platform::error() == ( int ) socket::error::would_block )
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


int
client::tls_setup()
{
    BIO *bio_err	= NULL;
	bool ok;
	int ret = ( int ) status::ok;

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
	int				ret = ( int ) status::ok;
	
	if ( !m_cert )
	{
		ret = tls_setup();
		
		if ( ret != ( int ) status::ok )
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
	int				ret = ( int ) status::ok;
	
	if ( !m_cert )
	{
		ret = tls_setup();
		
		if ( ret != ( int ) status::ok )
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
	if ( m_fd == socket::null )
	{
		m_fd = ::socket( AF_INET, SOCK_STREAM, 0 );
	}
	
	return ( m_fd != socket::null ) ? 0 : -1;
}


void
client::close()
{
	if ( m_connected )
	{
		::shutdown( m_fd, SHUT_RDWR );
		m_connected = false;
	}
	
	socket::client::close();
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
	
	return ( int ) ( ::send( self->m_fd, ( const std::uint8_t* ) str, ( int ) strlen( str ), 0 ) );
}


int
client::tls_read( BIO *h, char *buf, int size )
{
	client	*self = reinterpret_cast< client* >( h->ptr );
	int				ret;

	ret = ( int ) ::recv( self->m_fd, ( std::uint8_t* ) buf, size, 0 );

	return ret;
}


int
client::tls_write(BIO *h, const char *buf, int num )
{
	client	*self = reinterpret_cast< client* >( h->ptr );
	int				ret;

	ret = ( int ) ::send( self->m_fd, ( const std::uint8_t* ) buf, num, 0 );

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
	socket::server( addr->sockaddr().ss_family, SOCK_STREAM ),
	m_addr( addr )
{
}


server::~server()
{
}


int
server::open()
{
	if ( m_fd == socket::null )
	{
		m_fd = ::socket( AF_INET, SOCK_STREAM, 0 );
	}
	
	return ( m_fd != socket::null ) ? 0 : -1;
}


int
server::listen()
{
	sockaddr_storage	saddr;
	sockaddr_storage	saddr2;
	socklen_t			slen;
	runloop::source		listen_source;
	int					ret;
	
	memcpy( &saddr, &m_addr->sockaddr(), sizeof( saddr ) );
	
	ret = ::bind( m_fd, ( struct sockaddr* ) &saddr, saddr.ss_len );
	
	if ( ret != 0 )
	{
		nklog( log::error, "bind() failed: %d", platform::error() );
		goto exit;
	}
	
	slen = sizeof( saddr2 );
	ret = ::getsockname( m_fd, ( struct sockaddr* ) &saddr2, &slen );
	
	if ( ret != 0 )
	{
		nklog( log::error, "getsockname() failed: %d", platform::error() );
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
		nklog( log::error, "listen() failed: %d", platform::error() );
		goto exit;
	}
	
	listen_source = runloop::instance()->create_source( m_fd, netkit::runloop::event::read, [=]( runloop::source s, runloop::event event )
	{
		client::ptr			sock;
		ip::address::ptr	addr;
		
		sock = accept( addr );
		
		if ( sock )
		{
			auto source = runloop::instance()->create_source( sock->fd(), netkit::runloop::event::read, [=] ( runloop::source s, runloop::event event ) mutable
			{
				if ( sock->sink() )
				{
					sock->sink()->process();
				}
				else
				{
					uint8_t buf[ 64 ];
					ssize_t	num;
				
					num = sock->peek( buf, sizeof( buf ) );
				
					if ( num > 0 )
					{
						for ( adopters::list::iterator adopter = m_adopters.begin(); adopter != m_adopters.end(); adopter++ )
						{
							sink::ptr sink;
							
							sink = ( *adopter )( sock, buf, num );
	
							if ( sink )
							{
								break;
							}
						}
					}
				}
			} );
	
			runloop::instance()->schedule( source );
			
			sock->set_source( source );
		}
	} );
	
	runloop::instance()->schedule( listen_source );
	
exit:

	return ret;
}


client::ptr
server::accept( ip::address::ptr &addr )
{
	socket::native			newFd;
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

			newSock = new client( newFd, addr );
			
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

