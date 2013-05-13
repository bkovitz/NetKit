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
 */
// WebSocket, v1.00 2012-09-13
//
// Description: WebSocket FRC6544 codec, written in C++.
// Homepage: http://katzarsky.github.com/WebSocket
// Author: katzarsky@gmail.com

#include <NetKit/NKWebSocket.h>
#include <NetKit/NKRunLoop.h>
#include <NetKit/NKBase64.h>
#include <NetKit/NKSHA1.h>
#include <NetKit/NKLog.h>
#include <NetKit/NKPlatform.h>
#include <NetKit/NKError.h>
#include <assert.h>
#include <algorithm>
#include <stdio.h>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <memory>
#include <thread>


using namespace netkit;


class frame
{
public:

	enum class type
	{
		error				= 0xFF00,
		incomplete			= 0xFE00,

		opening				= 0x3300,
		closing				= 0x3400,

		incomplete_text		= 0x01,
		incomplete_binary	= 0x02,

		text				= 0x81,
		binary				= 0x82,

		ping				= 0x19,
		pong				= 0x1A
	};
};


class ws_adapter : public netkit::source::adapter
{
public:

	enum type
	{
		client,
		server
	};

	ws_adapter( type t );
	
	virtual ~ws_adapter();

	virtual void
	preflight( const uri::ref &uri, preflight_reply_f reply );
	
	virtual void
	connect( const uri::ref &uri, const endpoint::ref &to, connect_reply_f reply );
		
	virtual void
	send( const std::uint8_t *in_buf, std::size_t in_len, send_reply_f reply );
		
	virtual void
	recv( const std::uint8_t *in_buf, std::size_t in_len, recv_reply_f reply );
	
private:

	frame::type
	parse_server_handshake( const std::uint8_t *buf, std::size_t in_len, std::size_t *out_len );
	
	frame::type
	parse_client_handshake( unsigned char* input_frame, int input_len );

	std::string
	answer_client_handshake();

	int
	make_frame( frame::type type, unsigned char* msg, int msg_len, unsigned char* buffer, int buffer_len);

	frame::type
	get_frame( unsigned char* in_buffer, int in_length, unsigned char* out_buffer, int out_size, int* out_length);

	std::string
	trim( std::string str );

	std::vector< std::string >
	explode( std::string string, std::string delim, bool include_empty_strings = false );

	bool							m_handshake;
	std::vector< std::uint8_t >		m_handshake_data;
	std::string						m_expected_key;
	std::uint8_t					m_buffer[ 4192 ];
	std::vector< std::uint8_t >		m_send_data;
	bool							m_sending;
	std::vector< std::uint8_t >		m_unparsed_recv_data;
	std::vector< std::uint8_t >		m_parsed_recv_data;
	
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
		send_reply_f				m_reply;
		std::vector< std::uint8_t > m_data;

		inline buffer( send_reply_f reply, const std::uint8_t *data, std::size_t len )
		:
			m_reply( reply ),
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
	
	std::queue< buffer* >	m_pending_send_list;
	std::queue< buffer* >	m_pending_read_list;
	bool					m_read_required;
	bool					m_error;
	type					m_type;
	
	std::string m_resource;
	std::string m_host;
	std::string m_origin;
	std::string m_protocol;
	std::string m_key;

};


source::adapter::ref
ws::server::create()
{
	return new ws_adapter( ws_adapter::server );
}


source::adapter::ref
ws::client::create()
{
	return new ws_adapter( ws_adapter::client );
}


ws_adapter::ws_adapter( type t )
:
	m_read_required( false ),
	m_handshake( false ),
	m_sending( false ),
	m_error( false ),
	m_type( t )
{
	switch ( t )
	{
		case server:
		{
		}
		break;

		case client:
		{
		}
	}
}


ws_adapter::~ws_adapter()
{
}


void
ws_adapter::preflight( const uri::ref &uri, preflight_reply_f reply )
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
ws_adapter::connect( const uri::ref &uri, const endpoint::ref &to, connect_reply_f reply )
{
	m_next->connect( uri, to, [=]( int status ) mutable
	{
		if ( status == 0 )
		{
			uuid::ref			uuid = uuid::create();
			std::string			handshake;
			std::ostringstream	os;

			os << "GET " << uri->path() << "?encoding=text HTTP/1.1\r\n";
			os << "Upgrade: websocket\r\n";
			os << "Connection: Upgrade\r\n";
			os << "Host: " << uri->host() << "\r\n";
			os << "Sec-WebSocket-Key: " << uuid->to_base64() << "\r\n";
			os << "Sec-WebSocket-Version: 13\r\n\r\n";

			handshake = os.str();

			m_source->send( m_next, ( const std::uint8_t* ) handshake.c_str(), handshake.size(), [=]( int status )
			{
			} );
			
			m_expected_key = ws::server::accept_key( uuid->to_string( "" ) );
		}
		
		reply( status );
	} );
}


void
ws_adapter::send( const std::uint8_t *data, std::size_t len, send_reply_f reply )
{
	std::vector< std::uint8_t > raw( len * 2 );
	auto						actual = make_frame( frame::type::text, ( unsigned char* ) data, len, &raw[ 0 ], raw.size() );

	if ( actual > 0 )
	{
		if ( m_handshake )
		{
			m_next->send( &raw[ 0 ], actual, reply );
		}
		else
		{
			buffer *buf = new buffer( reply, &raw[ 0 ], actual );

			m_pending_send_list.push( buf );
		}
	}
	else
	{
		reply( -1, nullptr, 0 );
	}
}


void
ws_adapter::recv( const std::uint8_t *in_buf, std::size_t in_len, recv_reply_f reply )
{
	m_next->recv( in_buf, in_len, [=]( int status, const std::uint8_t *out_buf, std::size_t out_len )
	{
		if ( status == 0 )
		{
			frame::type type	= frame::type::incomplete;
			int			len		= 0;
			
			if ( m_handshake )
			{
				if ( out_len )
				{
					m_unparsed_recv_data.insert( m_unparsed_recv_data.begin(), out_buf, out_buf + out_len );
					m_parsed_recv_data.resize( m_unparsed_recv_data.size() );
				}
			}
			else
			{
				std::size_t out_header_len;
				
				m_unparsed_recv_data.insert( m_unparsed_recv_data.end(), out_buf, out_buf + out_len );
				
				type = parse_server_handshake( &m_unparsed_recv_data[ 0 ], m_unparsed_recv_data.size(), &out_header_len );
				
				if ( type == frame::type::opening )
				{
					m_handshake = true;
					
					if ( m_unparsed_recv_data.size() > out_header_len )
					{
						std::rotate( m_unparsed_recv_data.begin(), m_unparsed_recv_data.begin() + out_header_len, m_unparsed_recv_data.end() );
						m_unparsed_recv_data.resize( m_unparsed_recv_data.size() - out_header_len );
						m_parsed_recv_data.resize( m_unparsed_recv_data.size() );
					}
					else
					{
						m_unparsed_recv_data.clear();
					}
					
					while ( m_pending_send_list.size() > 0 )
					{
						buffer *b = m_pending_send_list.front();
						
						m_next->send( &b->m_data[ 0 ], b->m_data.size(), b->m_reply );
						
						m_pending_send_list.pop();
						
						delete b;
					}
				}
			}
			
			if ( ( type != frame::type::error ) && ( m_unparsed_recv_data.size() > 0 ) )
			{
				type = get_frame( &m_unparsed_recv_data[ 0 ], m_unparsed_recv_data.size(), &m_parsed_recv_data[ 0 ], m_parsed_recv_data.size(), &len );
			}
			
			if ( type == frame::type::text )
			{
				reply( 0, &m_parsed_recv_data[ 0 ], len );
				m_unparsed_recv_data.clear();
				m_parsed_recv_data.clear();
			}
			else if ( type != frame::type::error )
			{
				reply( 0, nullptr, 0 );
			}
			else
			{
				reply( -1, nullptr, 0 );
			}
		}
		else
		{
			reply( status, nullptr, 0 );
		}
	} );
}


frame::type
ws_adapter::parse_server_handshake( const std::uint8_t *buf, std::size_t in_len, std::size_t *out_len )
{
	std::string					header( ( char* ) buf, in_len );
	std::size_t					header_end = header.find( "\r\n\r\n" );
	std::vector< std::string >	lines;
	frame::type					type;

	if ( header_end == std::string::npos )
	{
		type = frame::type::incomplete;
		goto exit;
	}
	
	// trim off any data we don't need after the headers

	header.resize( header_end );

	lines = explode( header, std::string( "\r\n" ) );
	
	if ( lines.size() == 0 )
	{
		nklog( log::error, "no lines in websocket handshake header???" );
		type = frame::type::error;
		goto exit;
	}
	
	if ( lines[ 0 ] != "HTTP/1.1 101 Switching Protocols" )
	{
		nklog( log::error, "expecting 'HTTP/1.1 101 Switching Protocols', received %s", lines[ 0 ].c_str() );
		type = frame::type::error;
		goto exit;
	}
		
	for ( int i = 1; i < lines.size(); i++ )
	{
		std::string& line = lines[ i ];
		std::size_t pos = line.find( ":" );
		std::string key;
		std::string val;

		if ( pos == std::string::npos )
		{
			nklog( log::error, "malformed header" );
			type = frame::type::error;
			goto exit;
		}
		
		key = std::string( line, 0, pos );
		val = std::string( line, pos + 1 );

		val = trim( val );

		if ( key == "Sec-WebSocket-Accept" )
		{
			if ( m_expected_key != val )
			{
				nklog( log::error, "bad accept key in websocket handshake" );
				type = frame::type::error;
				goto exit;
			}
		}
	}
	
	*out_len = header_end + 4;
	
	type = frame::type::opening;
	
exit:

	return type;
}


frame::type
ws_adapter::parse_client_handshake( unsigned char* input_frame, int input_len )
{
	std::string headers( ( char* ) input_frame, input_len ); 
	std::size_t header_end = headers.find("\r\n\r\n");

	if ( header_end == std::string::npos )
	{
		// end-of-headers not found - do not parse
		return frame::type::incomplete;
	}

	// trim off any data we don't need after the headers

	headers.resize(header_end);

	std::vector< std::string > headers_rows = explode( headers, std::string( "\r\n" ) );

	for ( int i = 0; i < headers_rows.size(); i++ )
	{
		std::string& header = headers_rows[ i ];

		if ( header.find( "GET" ) == 0 )
		{
			std::vector< std::string > get_tokens = explode( header, std::string( " " ) );

			if ( get_tokens.size() >= 2 )
			{
				m_resource = get_tokens[ 1 ];
			}
		}
		else
		{
			std::size_t pos = header.find(":");

			if ( pos != std::string::npos )
			{
				std::string header_key( header, 0, pos );
				std::string header_value( header, pos + 1 );

				header_value = trim(header_value);

				if ( header_key == "Host" )
				{
					m_host = header_value;
				}
				else if ( header_key == "Origin" )
				{
					m_origin = header_value;
				}
				else if ( header_key == "Sec-WebSocket-Key" )
				{
					m_key = header_value;
				}
				else if ( header_key == "Sec-WebSocket-Protocol" )
				{
					m_protocol = header_value;
				}
			}
		}
	}

	//this->key = "dGhlIHNhbXBsZSBub25jZQ==";
	//printf("PARSED_KEY:%s \n", this->key.data());

	//return FrameType::OPENING_FRAME;
	printf("HANDSHAKE-PARSED\n");

	return frame::type::opening;
}


std::string
ws_adapter::trim( std::string str ) 
{
	const char* whitespace = " \t\r\n";
	std::string::size_type pos = str.find_last_not_of( whitespace );

	if ( pos != std::string::npos )
	{
		str.erase( pos + 1 );
		pos = str.find_first_not_of( whitespace );
		if ( pos != std::string::npos )
		{
			str.erase( 0, pos );
		}
	}
	else
	{
		return std::string();
	}

	return str;
}


std::vector< std::string >
ws_adapter::explode( std::string the_string, std::string delim, bool include_empty_strings )
{
	std::vector< std::string > strings;
	int start = 0;
	int end = 0;
	int length = 0;

	while ( end != std::string::npos )
	{
		end = the_string.find( delim, start );

		// If at end, use length=maxLength.  Else use length=end-start.
		length = (end == std::string::npos) ? std::string::npos : end - start;

		/* At end, end == length == string::npos */
		
		if (include_empty_strings || ( ( length > 0 ) && ( start  < the_string.size() ) ) )
		{
			strings.push_back( the_string.substr( start, length ) );
		}

		// If at end, use start=maxSize.  Else use start=end+delimiter.
		start = ( ( end > ( std::string::npos - delim.size() ) ) ? std::string::npos : end + delim.size() );
	}

	return strings;
}


std::string
ws_adapter::answer_client_handshake()
{
	std::string answer;

	answer += "HTTP/1.1 101 Switching Protocols\r\n";
	answer += "Upgrade: WebSocket\r\n";
	answer += "Connection: Upgrade\r\n";

	if ( m_key.length() > 0 )
	{
		answer += "Sec-WebSocket-Accept: "+ ws::server::accept_key( m_key ) + "\r\n";
	}

	if ( m_protocol.length() > 0 )
	{
		answer += "Sec-WebSocket-Protocol: "+ m_protocol + "\r\n";
	}

	answer += "\r\n";

	return answer;
}


int
ws_adapter::make_frame( frame::type type, unsigned char* msg, int msg_length, unsigned char* buffer, int buffer_size)
{
	int pos = 0;
	int size = msg_length; 
	buffer[ pos++ ] = (unsigned char)frame::type::text; // text frame

	if ( size<=125 )
	{
		buffer[ pos++ ] = size;
	}
	else if ( size<=65535 )
	{
		buffer[ pos++ ] = 126;					// 16 bit length
		buffer[ pos++ ] = ( size >> 8 ) & 0xFF; // rightmost first
		buffer[ pos++ ] = size & 0xFF;
	}
	else
	{
		// >2^16-1

		buffer[ pos++ ] = 127; //64 bit length

		//TODO: write 8 bytes length
		pos+=8;
	}

	memcpy( ( void* )( buffer+pos ), msg, size );

	return ( size+pos );
}


frame::type
ws_adapter::get_frame(unsigned char* in_buffer, int in_length, unsigned char* out_buffer, int out_size, int* out_length)
{
	if ( in_length < 3 )
	{
		return frame::type::incomplete;
	}

	unsigned char msg_opcode	= in_buffer[ 0 ] & 0x0F;
	unsigned char msg_fin		= ( in_buffer[ 0 ] >> 7 ) & 0x01;
	unsigned char msg_masked	= ( in_buffer[ 1 ] >> 7 ) & 0x01;

	// *** message decoding 

	int payload_length = 0;
	int pos = 2;
	int length_field = in_buffer[1] & (~0x80);
	unsigned int mask = 0;

	//printf("IN:"); for(int i=0; i<20; i++) printf("%02x ",buffer[i]); printf("\n");

	if ( length_field <= 125 )
	{
		payload_length = length_field;
	}
	else if ( length_field == 126 )
	{
		//msglen is 16bit!
		payload_length = in_buffer[2] + (in_buffer[3]<<8);
		pos += 2;
	}
	else if ( length_field == 127 )
	{
		//msglen is 64bit!
		payload_length = in_buffer[2] + (in_buffer[3]<<8); 
		pos += 8;
	}

	//printf("PAYLOAD_LEN: %08x\n", payload_length);
	if ( in_length < payload_length+pos )
	{
		return frame::type::incomplete;
	}

	if ( msg_masked )
	{
		mask = *( ( unsigned int* )( in_buffer + pos ) );
		//printf("MASK: %08x\n", mask);
		pos += 4;

		// unmask data:
		unsigned char* c = in_buffer + pos;

		for ( int i = 0; i < payload_length; i++ )
		{
			c[i] = c[i] ^ ((unsigned char*)(&mask))[i%4];
		}
	}

	if ( payload_length > out_size )
	{
		//TODO: if output buffer is too small -- ERROR or resize(free and allocate bigger one) the buffer ?
	}

	memcpy( ( void* ) out_buffer, ( void* )( in_buffer + pos ), payload_length );
	out_buffer[ payload_length ] = 0;
	*out_length = payload_length + 1;

	//printf("TEXT: %s\n", out_buffer);

	if (msg_opcode == 0x0 )
	{
		return ( msg_fin ) ? frame::type::text : frame::type::incomplete_text;// continuation frame ?
	}

	if( msg_opcode == 0x1 )
	{
		return ( msg_fin ) ? frame::type::text : frame::type::incomplete_text;
	}

	if ( msg_opcode == 0x2 )
	{
		return ( msg_fin ) ? frame::type::binary : frame::type::incomplete_binary;
	}

	if ( msg_opcode == 0x9 )
	{
		return frame::type::ping;
	}

	if ( msg_opcode == 0xA )
	{
		return frame::type::pong;
	}

	return frame::type::error;
}


std::string
ws::server::accept_key( const std::string &input )
{
	std::string			output( input );
    unsigned char		digest[ 20 ]; // 160 bit sha1 digest
	crypto::hash::sha1	sha;

	output += "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"; //RFC6544_MAGIC_KEY
	sha.input( output.data(),  output.size() );
	sha.result(( unsigned* ) digest );

	for ( int i = 0; i < 20; i += 4 )
	{
		unsigned char c;

		c = digest[i];
		digest[i] = digest[i+3];
		digest[i+3] = c;

		c = digest[i+1];
		digest[i+1] = digest[i+2];
		digest[i+2] = c;
	}

	output = codec::base64::encode( std::string( digest, digest + 20 ) ); //160bit = 20 bytes/chars
	
	return output;
}