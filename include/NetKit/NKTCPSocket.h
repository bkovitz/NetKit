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
 
#ifndef _netkit_tcp_socket_h
#define _netkit_tcp_socket_h

#include <NetKit/NKSocket.h>
#include <NetKit/NKIPAddress.h>
#include <openssl/ssl.h>
#include <string>
#include <deque>

namespace netkit {

namespace tcp {

class client;
typedef smart_ptr< client > client_ptr;

class server : public socket::server
{
public:

	typedef smart_ptr< server > ptr;
	typedef std::deque< ptr > list;

	server( const ip::address::ptr &addr );

	virtual ~server();
	
	virtual int
	open();
	
	inline uint16_t
	port() const
	{
		return m_addr->port();
	}
	
	virtual client_ptr
	accept( ip::address::ptr &addr );
	
protected:

	int
	listen();
	
	ip::address::ptr m_addr;
};


class client : public socket::client
{
public:

	typedef std::function< void ( int status ) > connect_reply_f;

	typedef smart_ptr< client > ptr;
	
	client();
	
	client( socket::native fd );
	
	client( socket::native fd, const ip::address::ptr &addr );
	
	virtual ~client();
	
	virtual int
	open();

	virtual void
	close();
	
	void
	connect( ip::address::ptr addr, connect_reply_f reply );
	
	virtual ssize_t
	peek( std::uint8_t *buf, size_t len );
	
	virtual ssize_t
	recv( std::uint8_t *buf, size_t len );
	
	virtual ssize_t
	send( const std::uint8_t *buf, size_t len );
	
	ip::address::ptr
	peer();
	
	int
	tls_connect();
	
	int
	tls_accept();
	
	inline bool
	connected() const
	{
		return m_connected;
	}
	
protected:

	struct sockaddr_storage	m_peer;
	socklen_t				m_peerLen;
	std::string				m_peerHost;
	std::string				m_peerEthernetAddr;
	bool					m_connected;
	
	static BIO_METHOD		m_tls_methods;
	static X509				*m_cert;
    static EVP_PKEY			*m_pkey;
	mutable SSL_CTX			*m_tls_context;
	mutable SSL				*m_tls;
	
private:

	int						tls_setup();
	static int				tls_write(BIO *h, const char *buf, int num);
	static int				tls_read(BIO *h, char *buf, int size);
	static int				tls_puts(BIO *h, const char *str);
	static long				tls_ctrl(BIO *h, int cmd, long arg1, void *arg2);
	static int				tls_new(BIO *h);
	static int				tls_free(BIO *data);
	bool					mkcert(X509 **x509p, EVP_PKEY **pkeyp, int bits, int serial, int days);
	int						add( X509 *cert, int nid, const char *value);
	static void				callback(int p, int n, void *arg);
};

}

}

#endif
