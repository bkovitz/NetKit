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

#include <NetKit/NKTLS.h>
#include <NetKit/NKRunLoop.h>
#include <NetKit/NKLog.h>
#include <NetKit/NKPlatform.h>
#include <NetKit/NKError.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/conf.h>
#include <openssl/x509v3.h>
#ifndef OPENSSL_NO_ENGINE
#include <openssl/engine.h>
#endif
#include <assert.h>
#include <algorithm>
#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <memory>
#include <thread>


using namespace netkit;


class tls_adapter : public netkit::source::adapter
{
public:

	enum type
	{
		client,
		server
	};

	tls_adapter( type t );
	
	virtual ~tls_adapter();

	virtual void
	preflight( const uri::ref &uri, preflight_reply_f reply );
	
	virtual void
	connect( const uri::ref &uri, const endpoint::ref &to, connect_reply_f reply );
		
	virtual void
	send( const std::uint8_t *in_buf, std::size_t in_len, send_reply_f reply );
		
	virtual void
	recv( const std::uint8_t *in_buf, std::size_t in_len, recv_reply_f reply );
	
private:
	
	bool
	make_cert( X509 **x509p, EVP_PKEY **pkeyp, int bits, int serial, int days );
	
	int
	add( X509 *cert, int nid, const char *value );
	
	static void
	callback(int p, int n, void *arg);
	
	bool							m_handshake;
	std::uint8_t					m_buffer[ 4192 ];
	std::vector< std::uint8_t >		m_send_data;
	bool							m_sending;
	std::vector< std::uint8_t >		m_recv_data;
	static X509						*m_cert;
	static EVP_PKEY					*m_pkey;
	
protected:

	void
	process();

	std::streamsize
	data_to_write( std::uint8_t *data, size_t len );

	std::streamsize
	data_to_read( std::uint8_t *data, size_t len );

protected:

	struct buffer
	{
		std::vector< std::uint8_t > m_data;

		inline buffer( const std::uint8_t *data, std::size_t len )
		:
			m_data( data, data + len )
		{
		}
	};
	
	inline void
	was_consumed( std::queue< buffer* > &queue, buffer *buf, std::size_t used )
	{
		if ( buf->m_data.size() == used )
		{
			queue.pop();
			delete buf;
		}
		else if ( used > 0 )
		{
			std::rotate( buf->m_data.begin(), buf->m_data.begin() + used, buf->m_data.end() );
			buf->m_data.resize( buf->m_data.size() - used );
		}
	}
	
	void
	send_pending_data();

	void
	handle_error( int result);

	std::queue< buffer* >	m_pending_write_list;
	std::queue< buffer* >	m_pending_read_list;
	bool					m_read_required;
	bool					m_error;
	SSL_CTX					*m_ssl_context;
	SSL						*m_ssl;
	BIO						*m_out;
	BIO						*m_in;
};


X509        *tls_adapter::m_cert	= NULL;
EVP_PKEY    *tls_adapter::m_pkey	= NULL;


source::adapter::ref
tls::server::create()
{
	return new tls_adapter( tls_adapter::server );
}


source::adapter::ref
tls::client::create()
{
	return new tls_adapter( tls_adapter::client );
}


tls_adapter::tls_adapter( type t )
:
	m_read_required( false ),
	m_handshake( false ),
	m_sending( false ),
	m_error( false ),
	m_ssl_context( nullptr ),
	m_ssl( nullptr ),
	m_out( nullptr ),
	m_in( nullptr )
{
	static bool init = false;
	
	if ( !init )
	{
		CRYPTO_malloc_init();
		SSL_load_error_strings();
		OpenSSL_add_all_algorithms();
		SSL_library_init();
		ENGINE_load_builtin_engines();
		
		auto ok = make_cert( &m_cert, &m_pkey, 512, 0, 3650 );
		
		if ( !ok )
		{
			nklog( log::error, "unable to make certificate" );
		}
	
#ifndef OPENSSL_NO_ENGINE
		ENGINE_cleanup();
#endif
		CRYPTO_cleanup_all_ex_data();
		
		init = true;
	}

	switch ( t )
	{
		case server:
		{
			m_ssl_context	= SSL_CTX_new( TLSv1_server_method() );
			SSL_CTX_set_options( m_ssl_context, SSL_OP_NO_SSLv2 );
			SSL_CTX_use_PrivateKey( m_ssl_context, m_pkey );
			SSL_CTX_use_certificate( m_ssl_context, m_cert );
	
			m_ssl	= SSL_new( m_ssl_context );
			m_in	= BIO_new( BIO_s_mem() );
			m_out	= BIO_new( BIO_s_mem() );
	
			SSL_set_bio( m_ssl, m_in, m_out );
			SSL_set_accept_state( m_ssl );
		}
		break;

		case client:
		{
			m_ssl_context	= SSL_CTX_new( TLSv1_client_method() );
			SSL_CTX_set_options( m_ssl_context, SSL_OP_NO_SSLv2 );
	
			m_ssl	= SSL_new( m_ssl_context );
			m_in	= BIO_new( BIO_s_mem() );
			m_out	= BIO_new( BIO_s_mem() );
			
			SSL_set_bio( m_ssl, m_in, m_out );
			SSL_set_connect_state( m_ssl );
		}
	}
}


tls_adapter::~tls_adapter()
{
	if ( m_out )
	{
		BIO_free( m_out );
	}
	
	if ( m_in )
	{
		BIO_free( m_in );
	}
	
	if ( m_ssl )
	{
		SSL_free( m_ssl );
	}
	
	if ( m_ssl_context )
	{
		SSL_CTX_free( m_ssl_context );
	}
}


void
tls_adapter::preflight( const uri::ref &uri, preflight_reply_f reply )
{
	if ( m_next )
	{
		m_next->preflight( uri, reply );
	}
	else
	{
		reply( 0, uri );
	}
}


void
tls_adapter::connect( const uri::ref &uri, const endpoint::ref &to, connect_reply_f reply )
{
	m_next->connect( uri, to, [=]( int status ) mutable
	{
		reply( status );
	} );
}


void
tls_adapter::send( const std::uint8_t *data, std::size_t len, send_reply_f reply )
{
	buffer *buf = new buffer( data, len );

	m_pending_write_list.push( buf );
	
	m_sending = true;

	m_send_data.clear();

	process();
	
	m_sending = false;
	
	if ( m_send_data.size() > 0 )
	{
		m_next->send( &m_send_data[ 0 ], m_send_data.size(), reply );
	}
	else if ( m_error )
	{
		reply( -1, nullptr, 0 );
	}
	else
	{
		reply( 0, nullptr, 0 );
	}
}


void
tls_adapter::recv( const std::uint8_t *in_buf, std::size_t in_len, recv_reply_f reply )
{
	m_next->recv( in_buf, in_len, [=]( int status, const std::uint8_t *out_buf, std::size_t out_len )
	{
		if ( status == 0 )
		{
			m_recv_data.clear();
			
			if ( out_len )
			{
				buffer *buf = new buffer( out_buf, out_len );

				m_pending_read_list.push( buf );

				process();
			}
		
			if ( m_recv_data.size() > 0 )
			{
				reply( 0, &m_recv_data[ 0 ], m_recv_data.size() );
			}
			else if ( m_error )
			{
				reply( -1, nullptr, 0 );
			}
			else
			{
				reply( 0, nullptr, 0 );
			}
		}
		else
		{
			reply( status, nullptr, 0 );
		}
	} );
}


void
tls_adapter::process()
{
	while ( ( !m_read_required && ( m_pending_write_list.size() > 0 ) ) || ( m_pending_read_list.size() > 0 ) )
	{
		if ( SSL_in_init( m_ssl ) )
		{
		}
		else if ( !m_handshake )
		{
			m_handshake = true;
		}

		if ( m_pending_read_list.size() > 0 )
		{
			buffer			*buf = m_pending_read_list.front();
			std::streamsize used = data_to_read( &buf->m_data[ 0 ], buf->m_data.size() );
			
			if ( used > 0 )
			{
				was_consumed( m_pending_read_list, buf, ( std::size_t ) used );
			}
			else if ( used < 0 )
			{
				break;
			}
		}

		if ( !m_read_required && ( m_pending_write_list.size() > 0 ) )
		{
			buffer			*buf = m_pending_write_list.front();
			
			std::streamsize used = data_to_write( &buf->m_data[ 0 ], buf->m_data.size() );
			
			if ( used > 0 )
			{
				was_consumed( m_pending_write_list, buf, ( std::size_t ) used );
			}
			else if ( used < 0 )
			{
				break;
			}
		}

		if ( BIO_ctrl_pending( m_out ) )
		{
			send_pending_data();
		}
	}
}


std::streamsize
tls_adapter::data_to_write( std::uint8_t *data, std::size_t len )
{
	std::streamsize bytes_used	= 0;
	std::streamsize result		= SSL_write( m_ssl, data, ( int ) len );

	if ( result < 0 )
	{
		handle_error( ( int ) result );
	}
	else
	{
		bytes_used = result;
	}

	if ( SSL_want_read( m_ssl ) )
	{
		m_read_required = true;
	}

	return bytes_used;
}


std::streamsize
tls_adapter::data_to_read( std::uint8_t *data, std::size_t len )
{
	std::uint8_t	buf[ 4192 ];
	std::size_t		bytes_used	= BIO_write( m_in, data, ( int ) len );
	int				bytes_out	= 0;
   
	m_read_required = false;
	
	do
	{
		bytes_out = SSL_read( m_ssl, ( void* ) buf, sizeof( buf ) );

		if ( bytes_out > 0 )
		{
			auto old_size = m_recv_data.size();
			m_recv_data.resize( m_recv_data.size() + bytes_out );
			memcpy( &m_recv_data[ old_size ], buf, bytes_out );
			
			
//			OnDataToRead( buf, bytes_out);
		}
      
		if ( bytes_out < 0 )
		{
			handle_error( bytes_out );
		}
	}
	while ( bytes_out > 0 );

	return bytes_used;
}


void
tls_adapter::send_pending_data()
{
	std::uint8_t	buf[ 4192 ];
	std::size_t		pending;

	while ( ( pending = BIO_ctrl_pending( m_out ) ) > 0 )
	{
		int bytes_to_send = BIO_read( m_out, ( void* ) buf, sizeof( buf ) );

		if ( bytes_to_send > 0 )
		{
//			OnDataToWrite( buf, bytes_to_send );

			if ( !m_sending )
			{
				m_source->send( m_next, buf, bytes_to_send, [=]( int status )
				{
				} );
			}
			else
			{
				auto old_size = m_send_data.size();
				m_send_data.resize( m_send_data.size() + bytes_to_send );
				memcpy( &m_send_data[ old_size ], buf, bytes_to_send );
			}
		}

		if ( bytes_to_send <= 0 )
		{
			if ( !BIO_should_retry( m_out ) )
			{
				handle_error(bytes_to_send);
			}
		}
	}
}


void
tls_adapter::handle_error( int result )
{
	if ( result <= 0 )
	{
		int error = SSL_get_error(m_ssl, result);

		switch ( error )
		{
			case SSL_ERROR_ZERO_RETURN:
			case SSL_ERROR_NONE:
			case SSL_ERROR_WANT_READ:
			{
			}
			break;

			default :
			{
				char buffer[256];

				while (error != 0)
				{
					ERR_error_string_n(error, buffer, sizeof(buffer));

					fprintf( stderr, "Error: %d - %s\n", error, buffer);

					error = ( int ) ERR_get_error();
				}
				
				//m_error = true;
			}
			break;
		}
	}
}


bool
tls_adapter::make_cert( X509 **x509p, EVP_PKEY **pkeyp, int bits, int serial, int days )
{
	X509		*x;
	EVP_PKEY	*pk;
	RSA			*rsa;
	X509_NAME	*name=NULL;
	bool		ok = false;
	
	if ( ( pkeyp == NULL ) || ( *pkeyp == NULL ) )
	{
		if ( ( pk = EVP_PKEY_new() ) == NULL )
		{
			nklog( log::error, "EVP_PKEY_new() failed" );
			goto exit;
		}
	}
	else
	{
		pk = *pkeyp;
	}

	if ( ( x509p == NULL ) || ( *x509p == NULL ) )
	{
		if ( ( x = X509_new() ) == NULL )
		{
			nklog( log::error, "X509_new() failed" );
			goto exit;
		}
	}
	else
	{
		x = *x509p;
	}

	rsa = RSA_generate_key( bits, RSA_F4, callback, NULL );
	
	if ( !EVP_PKEY_assign_RSA( pk, rsa ) )
	{
		nklog( log::error, "EVP_PKEY_assign_RSA() failed" );
		goto exit;
	}

	rsa = NULL;

	X509_set_version( x,2 );
	ASN1_INTEGER_set( X509_get_serialNumber(x),serial);
	X509_gmtime_adj( X509_get_notBefore(x),0);
	X509_gmtime_adj( X509_get_notAfter(x),(long)60*60*24*days);
	X509_set_pubkey( x,pk);

	name = X509_get_subject_name(x);

	if ( !X509_NAME_add_entry_by_txt(name,"C", MBSTRING_ASC, ( const unsigned char* ) "US", -1, -1, 0) )
	{
		nklog( log::error, "X509_NAME_add_entry_by_txt() failed" );
		goto exit;
	}
	
	if ( !X509_NAME_add_entry_by_txt(name,"CN", MBSTRING_ASC, ( const unsigned char* ) "OpenSSL Group", -1, -1, 0 ) )
	{
		nklog( log::error, "X509_NAME_add_entry_by_txt() failed" );
		goto exit;
	}

	// Its self signed so set the issuer name to be the same as the subject.
	
	X509_set_issuer_name(x,name);

	// Add various extensions: standard extensions
	
	if ( !add( x, NID_basic_constraints, "critical,CA:TRUE" ) )
	{
		goto exit;
	}
	
	if ( !add( x, NID_key_usage, "critical,keyCertSign,cRLSign" ) )
	{
		goto exit;
	}

	if ( !add( x, NID_subject_key_identifier, "hash" ) )
	{
		goto exit;
	}

	if ( !X509_sign( x, pk, EVP_md5() ) )
	{
		goto exit;
	}

	*x509p = x;
	*pkeyp = pk;
	
	ok = true;
	
exit:

	return ok;
}


int
tls_adapter::add( X509 *cert, int nid, const char *value )
{
	X509_EXTENSION *ex;
	X509V3_CTX		ctx;
	int				ret;
	
	X509V3_set_ctx_nodb(&ctx);
	X509V3_set_ctx(&ctx, cert, cert, NULL, NULL, 0);
	
	ex = X509V3_EXT_conf_nid(NULL, &ctx, nid, ( char* ) value);
	
	if ( !ex )
	{
		nklog( log::error, "X509V3_EXT_conf_nid() failed" );
		ret = 0;
		goto exit;
	}

	X509_add_ext(cert,ex,-1);
	X509_EXTENSION_free(ex);
	
	ret = 1;
	
exit:

	return ret;
}


void
tls_adapter::callback(int p, int n, void *arg)
{
	char c='B';

	if (p == 0) c='.';
	if (p == 1) c='+';
	if (p == 2) c='*';
	if (p == 3) c='\n';
	
	fputc( c,stderr );
}
