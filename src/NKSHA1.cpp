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
 *
 *  Copyright (C) 1998, 2009
 *  Paul E. Jones <paulej@packetizer.com>
 *  All Rights Reserved.
 *
 *****************************************************************************
 *  $Id: sha1.cpp 12 2009-06-22 19:34:25Z paulej $
 *****************************************************************************
 *
 *  Description:
 *      This class implements the Secure Hashing Standard as defined
 *      in FIPS PUB 180-1 published April 17, 1995.
 *
 *      The Secure Hashing Standard, which uses the Secure Hashing
 *      Algorithm (SHA), produces a 160-bit message digest for a
 *      given data stream.  In theory, it is highly improbable that
 *      two messages will produce the same message digest.  Therefore,
 *      this algorithm can serve as a means of providing a "fingerprint"
 *      for a message.
 *
 *  Portability Issues:
 *      SHA-1 is defined in terms of 32-bit "words".  This code was
 *      written with the expectation that the processor has at least
 *      a 32-bit machine word size.  If the machine word size is larger,
 *      the code should still function properly.  One caveat to that
 *      is that the input functions taking characters and character arrays
 *      assume that only 8 bits of information are stored in each character.
 *
 *  Caveats:
 *      SHA-1 is designed to work with messages less than 2^64 bits long.
 *      Although SHA-1 allows a message digest to be generated for
 *      messages of any number of bits less than 2^64, this implementation
 *      only works with messages with a length that is a multiple of 8
 *      bits.
 *
 */


#include <NetKit/NKSHA1.h>

using namespace netkit::crypto::hash;

sha1::sha1()
{
    reset();
}


sha1::~sha1()
{
    // The destructor does nothing
}


void
sha1::reset()
{
    m_length_low          = 0;
    m_length_high         = 0;
    m_message_block_index = 0;

    m_h[0]        = 0x67452301;
    m_h[1]        = 0xEFCDAB89;
    m_h[2]        = 0x98BADCFE;
    m_h[3]        = 0x10325476;
    m_h[4]        = 0xC3D2E1F0;

    m_computed    = false;
    m_corrupted   = false;
}


bool
sha1::result(unsigned *message_digest_array)
{
    int i;                                  // Counter

    if (m_corrupted)
    {
        return false;
    }

    if (!m_computed)
    {
        pad_message();
        m_computed = true;
    }

    for(i = 0; i < 5; i++)
    {
        message_digest_array[i] = m_h[i];
    }

    return true;
}


void
sha1::input( const unsigned char *message_array, unsigned length )
{
    if (!length)
    {
        return;
    }

    if (m_computed || m_corrupted)
    {
        m_corrupted = true;
        return;
    }

    while(length-- && !m_corrupted)
    {
        m_message_block[m_message_block_index++] = (*message_array & 0xFF);

        m_length_low += 8;
        m_length_low &= 0xFFFFFFFF;               // Force it to 32 bits
        if (m_length_low == 0)
        {
            m_length_high++;
            m_length_high &= 0xFFFFFFFF;          // Force it to 32 bits
            if (m_length_high == 0)
            {
                m_corrupted = true;               // Message is too long
            }
        }

        if (m_message_block_index == 64)
        {
            process_message_block();
        }

        message_array++;
    }
}


void
sha1::input( const char  *message_array, unsigned length )
{
	input((unsigned char *) message_array, length);
}


void
sha1::input(unsigned char message_element)
{
    input(&message_element, 1);
}


void
sha1::input( char message_element )
{
	input((unsigned char *) &message_element, 1);
}


sha1&
sha1::operator<<(const char *message_array)
{
    const char *p = message_array;

    while(*p)
    {
        input(*p);
        p++;
    }

    return *this;
}


sha1&
sha1::operator<<(const unsigned char *message_array)
{
    const unsigned char *p = message_array;

    while(*p)
    {
        input(*p);
        p++;
    }

    return *this;
}


sha1&
sha1::operator<<(const char message_element)
{
    input((unsigned char *) &message_element, 1);

    return *this;
}


sha1&
sha1::operator<<(const unsigned char message_element)
{
    input(&message_element, 1);

    return *this;
}


void
sha1::process_message_block()
{
    const unsigned K[] =    {               // Constants defined for SHA-1
                                0x5A827999,
                                0x6ED9EBA1,
                                0x8F1BBCDC,
                                0xCA62C1D6
                            };
    int         t;                          // Loop counter
    unsigned    temp;                       // Temporary word value
    unsigned    W[80];                      // Word sequence
    unsigned    A, B, C, D, E;              // Word buffers

    /*
     *  Initialize the first 16 words in the array W
     */
    for(t = 0; t < 16; t++)
    {
        W[t] = ((unsigned) m_message_block[t * 4]) << 24;
        W[t] |= ((unsigned) m_message_block[t * 4 + 1]) << 16;
        W[t] |= ((unsigned) m_message_block[t * 4 + 2]) << 8;
        W[t] |= ((unsigned) m_message_block[t * 4 + 3]);
    }

    for(t = 16; t < 80; t++)
    {
       W[t] = circular_shift(1,W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16]);
    }

    A = m_h[0];
    B = m_h[1];
    C = m_h[2];
    D = m_h[3];
    E = m_h[4];

    for(t = 0; t < 20; t++)
    {
        temp = circular_shift(5,A) + ((B & C) | ((~B) & D)) + E + W[t] + K[0];
        temp &= 0xFFFFFFFF;
        E = D;
        D = C;
        C = circular_shift(30,B);
        B = A;
        A = temp;
    }

    for(t = 20; t < 40; t++)
    {
        temp = circular_shift(5,A) + (B ^ C ^ D) + E + W[t] + K[1];
        temp &= 0xFFFFFFFF;
        E = D;
        D = C;
        C = circular_shift(30,B);
        B = A;
        A = temp;
    }

    for(t = 40; t < 60; t++)
    {
        temp = circular_shift(5,A) +
               ((B & C) | (B & D) | (C & D)) + E + W[t] + K[2];
        temp &= 0xFFFFFFFF;
        E = D;
        D = C;
        C = circular_shift(30,B);
        B = A;
        A = temp;
    }

    for(t = 60; t < 80; t++)
    {
        temp = circular_shift(5,A) + (B ^ C ^ D) + E + W[t] + K[3];
        temp &= 0xFFFFFFFF;
        E = D;
        D = C;
        C = circular_shift(30,B);
        B = A;
        A = temp;
    }

    m_h[0] = (m_h[0] + A) & 0xFFFFFFFF;
    m_h[1] = (m_h[1] + B) & 0xFFFFFFFF;
    m_h[2] = (m_h[2] + C) & 0xFFFFFFFF;
    m_h[3] = (m_h[3] + D) & 0xFFFFFFFF;
    m_h[4] = (m_h[4] + E) & 0xFFFFFFFF;

    m_message_block_index = 0;
}


void
sha1::pad_message()
{
    /*
     *  Check to see if the current message block is too small to hold
     *  the initial padding bits and length.  If so, we will pad the
     *  block, process it, and then continue padding into a second block.
     */
    if (m_message_block_index > 55)
    {
        m_message_block[m_message_block_index++] = 0x80;
        while(m_message_block_index < 64)
        {
            m_message_block[m_message_block_index++] = 0;
        }

        process_message_block();

        while(m_message_block_index < 56)
        {
            m_message_block[m_message_block_index++] = 0;
        }
    }
    else
    {
        m_message_block[m_message_block_index++] = 0x80;
        while(m_message_block_index < 56)
        {
            m_message_block[m_message_block_index++] = 0;
        }

    }

    /*
     *  Store the message length as the last 8 octets
     */
    m_message_block[56] = (m_length_high >> 24) & 0xFF;
    m_message_block[57] = (m_length_high >> 16) & 0xFF;
    m_message_block[58] = (m_length_high >> 8) & 0xFF;
    m_message_block[59] = (m_length_high) & 0xFF;
    m_message_block[60] = (m_length_low >> 24) & 0xFF;
    m_message_block[61] = (m_length_low >> 16) & 0xFF;
    m_message_block[62] = (m_length_low >> 8) & 0xFF;
    m_message_block[63] = (m_length_low) & 0xFF;

    process_message_block();
}


unsigned
sha1::circular_shift(int bits, unsigned word)
{
    return ((word << bits) & 0xFFFFFFFF) | ((word & 0xFFFFFFFF) >> (32-bits));
}