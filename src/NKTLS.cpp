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
#include <sys/socket.h>
#include <botan_all.h>
#include <iostream>
#include <fstream>
#include <memory>
#include <thread>


using namespace netkit;
using namespace Botan;



bool value_exists(const std::vector<std::string>& vec,
                  const std::string& val)
   {
   for(size_t i = 0; i != vec.size(); ++i)
      if(vec[i] == val)
         return true;
   return false;
   }

class Credentials_Manager_Simple : public Botan::Credentials_Manager
{
public:

	Credentials_Manager_Simple( Botan::RandomNumberGenerator &rng )
	:
		rng( rng )
	{
	}

	inline std::string
	srp_identifier( const std::string& type, const std::string& hostname )
	{
		if ( type == "tls-client" && hostname == "srp-host" )
		{
			return "user";
		}
		
		return "";
	}

	inline bool
	attempt_srp( const std::string& type, const std::string& hostname )
	{
		if ( hostname == "srp-host" )
		{
			return true;
		}

		return false;
	}

	inline std::vector< Botan::Certificate_Store* >
	trusted_certificate_authorities( const std::string& type, const std::string& hostname )
	{
		std::vector<Botan::Certificate_Store*> certs;

		if ( type == "tls-client" && hostname == "twitter.com" )
		{
			Botan::X509_Certificate verisign("/usr/share/ca-certificates/mozilla/VeriSign_Class_3_Public_Primary_Certification_Authority_-_G5.crt");

			auto store = new Botan::Certificate_Store_In_Memory;
			store->add_certificate(verisign);
			certs.push_back(store);
		}

		return certs;
	}

      void verify_certificate_chain(
         const std::string& type,
         const std::string& purported_hostname,
         const std::vector<Botan::X509_Certificate>& cert_chain)
         {
         try
            {
            Botan::Credentials_Manager::verify_certificate_chain(type,
                                                                 purported_hostname,
                                                                 cert_chain);
            }
         catch(std::exception& e)
            {
            std::cout << "Certificate verification failed - " << e.what() << " - but will ignore\n";
            }
         }

      std::string srp_password(const std::string& type,
                               const std::string& hostname,
                               const std::string& identifier)
         {
         if(type == "tls-client" && hostname == "localhost" && identifier == "user")
            return "password";

         return "";
         }

      bool srp_verifier(const std::string& type,
                        const std::string& context,
                        const std::string& identifier,
                        std::string& group_id,
                        Botan::BigInt& verifier,
                        std::vector<Botan::byte>& salt,
                        bool generate_fake_on_unknown)
         {

         std::string pass = srp_password("tls-client", context, identifier);
         if(pass == "")
            {
            if(!generate_fake_on_unknown)
               return false;

            pass.resize(16);
            Botan::global_state().global_rng().randomize((Botan::byte*)&pass[0], pass.size());
            }

         group_id = "modp/srp/2048";

         salt.resize(16);
         Botan::global_state().global_rng().randomize(&salt[0], salt.size());

         verifier = Botan::generate_srp6_verifier(identifier,
                                                  pass,
                                                  salt,
                                                  group_id,
                                                  "SHA-1");

         return true;
         }

      std::string psk_identity_hint(const std::string&,
                                    const std::string&)
         {
         return "";
         }

      std::string psk_identity(const std::string&, const std::string&,
                               const std::string& identity_hint)
         {
         //return "lloyd";
         return "Client_identity";
         }

      Botan::SymmetricKey psk(const std::string& type, const std::string& context,
                              const std::string& identity)
         {
         if(type == "tls-server" && context == "session-ticket")
            {
            if(session_ticket_key.length() == 0)
               session_ticket_key = Botan::SymmetricKey(rng, 32);
            return session_ticket_key;
            }

         if(identity == "Client_identity")
            return Botan::SymmetricKey("b5a72e1387552e6dc10766dc0eda12961f5b21e17f98ef4c41e6572e53bd7527");
         if(identity == "lloyd")
            return Botan::SymmetricKey("85b3c1b7dc62b507636ac767999c9630");

         throw Botan::Internal_Error("No PSK set for " + identity);
         }

      std::pair<Botan::X509_Certificate,Botan::Private_Key*>
      load_or_make_cert(const std::string& hostname,
                        const std::string& key_type,
                        Botan::RandomNumberGenerator& rng)
         {
         using namespace Botan;

         const std::string key_fsname_prefix = hostname + "." + key_type + ".";
         const std::string key_file_name = key_fsname_prefix + "key";
         const std::string cert_file_name = key_fsname_prefix + "crt";

         try
            {
            X509_Certificate cert(cert_file_name);
            Private_Key* key = PKCS8::load_key(key_file_name, rng);

            //std::cout << "Loaded existing key/cert from " << cert_file_name << " and " << key_file_name << "\n";

            return std::make_pair(cert, key);
            }
         catch(...) {}

         // Failed. Instead, make a new one

         std::cout << "Creating new certificate for identifier '" << hostname << "'\n";

         X509_Cert_Options opts;

         opts.common_name = hostname;
         opts.country = "US";
         opts.email = "root@" + hostname;
         opts.dns = hostname;

         std::auto_ptr<Private_Key> key;
         if(key_type == "rsa")
            key.reset(new RSA_PrivateKey(rng, 1024));
         else if(key_type == "dsa")
            key.reset(new DSA_PrivateKey(rng, DL_Group("dsa/jce/1024")));
         else if(key_type == "ecdsa")
            key.reset(new ECDSA_PrivateKey(rng, EC_Group("secp256r1")));
         else
            throw std::runtime_error("Don't know what to do about key type '" + key_type + "'");

         X509_Certificate cert =
            X509::create_self_signed_cert(opts, *key, "SHA-1", rng);

         // Now save both

         std::cout << "Saving new " << key_type << " key to " << key_file_name << "\n";
         std::ofstream key_file(key_file_name.c_str());
         key_file << PKCS8::PEM_encode(*key, rng, "");
         key_file.close();

         std::cout << "Saving new " << key_type << " cert to " << key_file_name << "\n";
         std::ofstream cert_file(cert_file_name.c_str());
         cert_file << cert.PEM_encode() << "\n";
         cert_file.close();

         return std::make_pair(cert, key.release());
         }

      std::vector<Botan::X509_Certificate> cert_chain(
         const std::vector<std::string>& cert_key_types,
         const std::string& type,
         const std::string& context)
         {
         using namespace Botan;

         std::vector<X509_Certificate> certs;

         try
            {
            if(type == "tls-server")
               {
               const std::string hostname = (context == "" ? "localhost" : context);

               if(hostname == "nosuchname")
                  return std::vector<Botan::X509_Certificate>();

               for(auto i : certs_and_keys)
                  {
                  if(hostname != "" && !i.first.matches_dns_name(hostname))
                     continue;

                  if(!value_exists(cert_key_types, i.second->algo_name()))
                     continue;

                  certs.push_back(i.first);
                  }

               if(!certs.empty())
                  return certs;

               std::string key_name = "";

               if(value_exists(cert_key_types, "RSA"))
                  key_name = "rsa";
               else if(value_exists(cert_key_types, "DSA"))
                  key_name = "dsa";
               else if(value_exists(cert_key_types, "ECDSA"))
                  key_name = "ecdsa";

               std::pair<X509_Certificate, Private_Key*> cert_and_key =
                  load_or_make_cert(hostname, key_name, rng);

               certs_and_keys[cert_and_key.first] = cert_and_key.second;
               certs.push_back(cert_and_key.first);
               }
            else if(type == "tls-client")
               {
               X509_Certificate cert("user-rsa.crt");
               Private_Key* key = PKCS8::load_key("user-rsa.key", rng);

               certs_and_keys[cert] = key;
               certs.push_back(cert);
               }
            }
         catch(std::exception& e)
            {
            std::cout << e.what() << "\n";
            }

         return certs;
         }

      Botan::Private_Key* private_key_for(const Botan::X509_Certificate& cert,
                                          const std::string& type,
                                          const std::string& context)
         {
         return certs_and_keys[cert];
         }

   private:
      Botan::RandomNumberGenerator& rng;

      Botan::SymmetricKey session_ticket_key;

      std::map<Botan::X509_Certificate, Botan::Private_Key*> certs_and_keys;
};


class tls_impl : public netkit::tls::adapter
{
public:

	tls_impl();
	
	virtual ~tls_impl();

	virtual void
	accept( source::accept_reply_f reply );
		
	virtual void
	preflight( const uri::ptr &uri, preflight_reply_f reply );
	
	virtual void
	connect( const uri::ptr &uri, const endpoint::ptr &to, connect_reply_f reply );
		
	virtual void
	send( const std::uint8_t *in_buf, std::size_t in_len, send_reply_f reply );
		
	virtual void
	recv( const std::uint8_t *in_buf, std::size_t in_len, recv_reply_f reply );
	
private:
	
	void
	accept_internal( accept_reply_f reply );
	
	void
	connect_internal( const uri::ptr &uri, const endpoint::ptr &to, connect_reply_f reply );
	
	std::string
	protocol_chooser( const std::vector< std::string > &protocols );
	
	void
	wait_for_handshake();
	
	bool							m_handshake;
	TLS::Client						*m_client;
	TLS::Server						*m_server;
	std::uint8_t					m_buffer[ 4192 ];
	std::vector< std::uint8_t >		m_send_data;
	std::vector< std::uint8_t >		m_recv_data;
	AutoSeeded_RNG					m_rng;
	TLS::Policy						m_policy;
	TLS::Session_Manager_In_Memory	m_session;
	Credentials_Manager_Simple		m_creds;
};


tls::adapter::ptr
tls::adapter::create()
{
	return new tls_impl;
}


tls_impl::tls_impl()
:
	m_client( nullptr ),
	m_server( nullptr ),
	m_handshake( false ),
	m_creds( m_rng )
{
}


tls_impl::~tls_impl()
{
	if ( m_client )
	{
		delete m_client;
	}
	
	if ( m_server )
	{
		delete m_server;
	}
}


void
tls_impl::accept( source::accept_reply_f reply )
{
	if ( m_next )
	{
		m_next->accept( [=]( int status )
		{
			if ( status == 0 )
			{
				accept_internal( reply );
			}
			else
			{
				reply( status );
			}
		} );
	}
	else
	{
		accept_internal( reply );
	}
}


void
tls_impl::accept_internal( source::accept_reply_f reply )
{
	m_server = new TLS::Server
					(
					[=]( const byte *buf, std::size_t len ) mutable
					{
						// We need to make sure this send completes
						
						m_source->send( m_next, buf, len, [=]( int status )
						{
						} );
					},
								
					[=]( const byte *buf, std::size_t len, TLS::Alert alert )
					{
						if ( alert.is_valid() )
						{
							nklog( log::info, "tls alert: %s", alert.type_string().c_str() );
						}
									
						auto old_size = m_recv_data.size();
						m_recv_data.resize( m_recv_data.size() + len );
						memcpy( &m_recv_data[ old_size ], buf, len );
					},
								
					[=]( const TLS::Session &session )
					{
						// Don't reply right now, because Botan won't finish initializing this
						// connection until we've returned true.
									
						runloop::instance()->dispatch_on_main_thread( [=]()
						{
							reply( 0 );
						} );
									
						return true;
					},
								
					m_session,
					m_creds,
					m_policy,
					m_rng
					);
}


void
tls_impl::preflight( const uri::ptr &uri, preflight_reply_f reply )
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
tls_impl::connect( const uri::ptr &uri, const endpoint::ptr &to, connect_reply_f reply )
{
	if ( m_prev )
	{
		m_prev->connect( uri, to, [=]( int status ) mutable
		{
			if ( status == 0 )
			{
				connect_internal( uri, to, reply );
			}
			else
			{
				reply( status );
			}
		} );
	}
	else
	{
		connect_internal( uri, to, reply );
	}
}


void
tls_impl::connect_internal( const uri::ptr &uri, const endpoint::ptr &to, connect_reply_f reply )
{
	m_client = new TLS::Client
					(
					[=]( const byte *buf, std::size_t len ) mutable
					{
						m_source->send( m_next, buf, len, [=]( int status )
						{
							// We need to make sure this send completes
							
							if ( status != 0 )
							{
							}
						} );
					},
								
					[=]( const byte *buf, std::size_t len, TLS::Alert alert )
					{
						if ( alert.is_valid() )
						{
							nklog( log::info, "tls alert: %s", alert.type_string().c_str() );
						}
									
						auto old_size = m_recv_data.size();
						m_recv_data.resize( m_recv_data.size() + len );
						memcpy( &m_recv_data[ old_size ], buf, len );
					},
								
					[=]( const TLS::Session &session )
					{
						// Don't reply right now, because Botan won't finish initializing this
						// connection until we've returned true.
									
						m_handshake = true;
									
						runloop::instance()->dispatch_on_main_thread( [=]()
						{
							reply( 0 );
						} );
									
						return true;
					},
								
					m_session,
					m_creds,
					m_policy,
					m_rng,
					TLS::Server_Information( uri->host(), uri->port() ),
					TLS::Protocol_Version::latest_tls_version(),
					std::bind( &tls_impl::protocol_chooser, this, std::placeholders::_1 )
					);
				
	wait_for_handshake();
}


void
tls_impl::recv( const std::uint8_t *in_buf, std::size_t in_len, recv_reply_f reply )
{
	m_next->recv( in_buf, in_len, [=]( int status, const std::uint8_t *buf, std::size_t len )
	{
		if ( m_client )
		{
			m_client->received_data( buf, len );
		}
		else if ( m_server )
		{
			m_server->received_data( buf, len );
		}
	
		if ( m_recv_data.size() > 0 )
		{
			reply( 0, &m_recv_data[ 0 ], m_recv_data.size() );
			m_recv_data.clear();
		}
	} );
}


void
tls_impl::send( const std::uint8_t *buf, std::size_t len, send_reply_f reply )
{
	try
	{
		if ( m_client )
		{
			m_client->send( buf, len );
		}
		else if ( m_server )
		{
			m_server->send( buf, len );
		}
	}
	catch ( ... )
	{
		nklog( log::error, "caught exception when sending tls data" );
		len = -1;
	}
}

	
std::string
tls_impl::protocol_chooser( const std::vector< std::string > &protocols )
{
	for ( auto it = protocols.begin(); it != protocols.end(); it++ )
	{
		fprintf( stderr, "protocol: %s\n", ( *it ).c_str() );
	}
	
	return "http/1.1";
}


void
tls_impl::wait_for_handshake()
{
	if ( !m_handshake )
	{
		m_source->recv( nullptr, 0, [=]( int status, size_t len )
		{
			wait_for_handshake();
		} );
	}
}