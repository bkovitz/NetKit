#ifndef _CoreApp_tcp_socket_h
#define _CoreApp_tcp_socket_h

#include "socket.h"
#include <CoreApp/ip_address.h>
#include <CoreApp/server.h>
#include <CoreApp/tstring.h>
#include <CoreApp/types.h>
#include <openssl/ssl.h>
#include <deque>

namespace CoreApp {
namespace tcp {

class client : public socket
{
public:

	typedef std::function< void ( int status ) > connect_reply;

	typedef smart_ptr< client > ptr;
	
	client();
	
	client( native fd );
	
	virtual ~client();
	
	virtual int
	open();

	virtual void
	close();
	
	void
	connect( ip::address::ptr addr, connect_reply reply );
	
	ssize_t
	send( const uint8_t *buf, size_t len ) const;
	
	ssize_t
	recv( uint8_t *buf, size_t len ) const;
	
	ssize_t
	peek( uint8_t *buf, size_t len ) const;
	
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


class server : public socket
{
public:

	typedef smart_ptr< server > ptr;
	typedef std::deque< ptr > list;

	server( const ip::address::ptr &addr );

	virtual ~server();
	
	virtual int
	open();
	
	void
	add_listener( CoreApp::server::ptr s );
	
	client::ptr
	accept( ip::address::ptr &addr );
	
	inline uint16_t
	port() const
	{
		return m_addr->port();
	}
	
protected:

	typedef std::deque< CoreApp::server::ptr > listeners;
	
	int
	listen();
	
	listeners			m_listeners;
	ip::address::ptr	m_addr;
};

}
}

#endif
