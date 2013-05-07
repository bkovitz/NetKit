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
/*
 *  Copyright (C) 1998, 2009
 *  Paul E. Jones <paulej@packetizer.com>
 *  All Rights Reserved.
 *
 *****************************************************************************
 *  $Id: sha1.h 12 2009-06-22 19:34:25Z paulej $
 *****************************************************************************
 *
 *  Description:
 *      This class implements the Secure Hashing Standard as defined
 *      in FIPS PUB 180-1 published April 17, 1995.
 *
 *      Many of the variable names in this class, especially the single
 *      character names, were used because those were the names used
 *      in the publication.
 *
 *      Please read the file sha1.cpp for more information.
 *
 */

#ifndef _netkit_sha1_h
#define _netkit_sha1_h

#include <NetKit/NKObject.h>

namespace netkit {

namespace crypto {

namespace hash {

class sha1
{
public:

	sha1();

	virtual ~sha1();

	void
	reset();

	bool
	result( unsigned *message_digest_array );

	void
	input( const unsigned char *message_array, unsigned length );

	void
	input( const char *message_array, unsigned length );

	void
	input( unsigned char message_element );

	void
	input( char message_element );

	sha1&
	operator<<(const char *message_array);

	sha1&
	operator<<(const unsigned char *message_array);

	sha1&
	operator<<(const char message_element);

	sha1&
	operator<<(const unsigned char message_element);

private:

	void
	process_message_block();

	void
	pad_message();

	inline unsigned
	circular_shift( int bits, unsigned word );

	unsigned		m_h[5];						// Message digest buffers

	unsigned		m_length_low;				// Message length in bits
	unsigned		m_length_high;				// Message length in bits

	unsigned char	m_message_block[64];		// 512-bit message blocks
	int				m_message_block_index;		// Index into message block array

	bool			m_computed;					// Is the digest computed?
	bool			m_corrupted;				// Is the message digest corruped?
};

}

}

}

#endif