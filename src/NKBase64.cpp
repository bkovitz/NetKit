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
 
#include <NetKit/NKBase64.h>
#include <stdlib.h>

//
//  Created by Matt Gallagher on 2009/06/03.
//  Copyright 2009 Matt Gallagher. All rights reserved.
//
//  This software is provided 'as-is', without any express or implied
//  warranty. In no event will the authors be held liable for any damages
//  arising from the use of this software. Permission is granted to anyone to
//  use this software for any purpose, including commercial applications, and to
//  alter it and redistribute it freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you must not
//     claim that you wrote the original software. If you use this software
//     in a product, an acknowledgment in the product documentation would be
//     appreciated but is not required.
//  2. Altered source versions must be plainly marked as such, and must not be
//     misrepresented as being the original software.
//  3. This notice may not be removed or altered from any source
//     distribution.
//

//
// Mapping from 6 bit pattern to ASCII character.
//

static unsigned char base64_encode_lookup[ 65 ] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

//
// Definition for "masked-out" areas of the base64_decode_lookup mapping
//
#define xx 65

//
// Mapping from ASCII character to 6 bit pattern.
//
static unsigned char base64_decode_lookup[256] =
{
    xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, 
    xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, 
    xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, 62, xx, xx, xx, 63, 
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, xx, xx, xx, xx, xx, xx, 
    xx,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, xx, xx, xx, xx, xx, 
    xx, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, xx, xx, xx, xx, xx, 
    xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, 
    xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, 
    xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, 
    xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, 
    xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, 
    xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, 
    xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, 
    xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, 
};

//
// Fundamental sizes of the binary and base64 encode/decode units in bytes
//
#define BINARY_UNIT_SIZE 3
#define BASE64_UNIT_SIZE 4


#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

using namespace netkit::codec;


std::string
base64::encode( const std::string &s )
{
	#define MAX_NUM_PADDING_CHARS 2
	#define OUTPUT_LINE_LENGTH 64
	#define INPUT_LINE_LENGTH ((OUTPUT_LINE_LENGTH / BASE64_UNIT_SIZE) * BINARY_UNIT_SIZE)
	#define CR_LF_SIZE 2

	const unsigned char *inputBuffer = ( const unsigned char* ) s.c_str();
	bool separateLines = true;
	size_t i = 0;
	size_t j = 0;
	const size_t lineLength = separateLines ? INPUT_LINE_LENGTH : s.size();
	size_t lineEnd = lineLength;
	std::string encoded;
	
	//
	// Byte accurate calculation of final buffer size
	//
	size_t outputBufferSize = ( ( s.size() / BINARY_UNIT_SIZE ) + ( ( s.size() % BINARY_UNIT_SIZE ) ? 1 : 0 ) ) * BASE64_UNIT_SIZE;
	
	if ( separateLines )
	{
		outputBufferSize += ( outputBufferSize / OUTPUT_LINE_LENGTH ) * CR_LF_SIZE;
	}
	
	//
	// Include space for a terminating zero
	//
	outputBufferSize += 1;

	//
	// Allocate the output buffer
	//
	char *outputBuffer = ( char* ) malloc( outputBufferSize );
	
	if ( !outputBuffer )
	{
		goto exit;
	}
	
	while (true)
	{
		if (lineEnd > s.size() )
		{
			lineEnd = s.size();
		}

		for (; i + BINARY_UNIT_SIZE - 1 < lineEnd; i += BINARY_UNIT_SIZE)
		{
			//
			// Inner loop: turn 48 bytes into 64 base64 characters
			//
			outputBuffer[j++] = base64_encode_lookup[(inputBuffer[i] & 0xFC) >> 2];
			outputBuffer[j++] = base64_encode_lookup[((inputBuffer[i] & 0x03) << 4)
				| ((inputBuffer[i + 1] & 0xF0) >> 4)];
			outputBuffer[j++] = base64_encode_lookup[((inputBuffer[i + 1] & 0x0F) << 2)
				| ((inputBuffer[i + 2] & 0xC0) >> 6)];
			outputBuffer[j++] = base64_encode_lookup[inputBuffer[i + 2] & 0x3F];
		}
		
		if (lineEnd == s.size() )
		{
			break;
		}
		
		//
		// Add the newline
		//
		outputBuffer[j++] = '\r';
		outputBuffer[j++] = '\n';
		lineEnd += lineLength;
	}
	
	if (i + 1 < s.size() )
	{
		//
		// Handle the single '=' case
		//
		outputBuffer[j++] = base64_encode_lookup[(inputBuffer[i] & 0xFC) >> 2];
		outputBuffer[j++] = base64_encode_lookup[((inputBuffer[i] & 0x03) << 4)
			| ((inputBuffer[i + 1] & 0xF0) >> 4)];
		outputBuffer[j++] = base64_encode_lookup[(inputBuffer[i + 1] & 0x0F) << 2];
		outputBuffer[j++] =	'=';
	}
	else if (i < s.size() )
	{
		//
		// Handle the double '=' case
		//
		outputBuffer[j++] = base64_encode_lookup[(inputBuffer[i] & 0xFC) >> 2];
		outputBuffer[j++] = base64_encode_lookup[(inputBuffer[i] & 0x03) << 4];
		outputBuffer[j++] = '=';
		outputBuffer[j++] = '=';
	}

	outputBuffer[j] = 0;
	
	//
	// Set the output length and return the buffer
	//

	encoded = outputBuffer;
	
exit:

	if ( outputBuffer )
	{
		free( outputBuffer );
	}

	return encoded;
}


std::string
base64::decode( const std::string &s )
{
	size_t			length = s.size();
	const char		*inputBuffer		= s.c_str();
	size_t			outputBufferSize	= ( ( length + BASE64_UNIT_SIZE - 1 ) / BASE64_UNIT_SIZE ) * BINARY_UNIT_SIZE;
	unsigned char	*outputBuffer		= ( unsigned char* ) malloc(outputBufferSize);
	std::string decoded;
	
	size_t i = 0;
	size_t j = 0;

	while ( i < length )
	{
		//
		// Accumulate 4 valid characters (ignore everything else)
		//
		unsigned char accumulated[BASE64_UNIT_SIZE];
		size_t accumulateIndex = 0;
		while (i < length)
		{
			unsigned char decode = base64_decode_lookup[inputBuffer[i++]];
			if (decode != xx)
			{
				accumulated[accumulateIndex] = decode;
				accumulateIndex++;
				
				if (accumulateIndex == BASE64_UNIT_SIZE)
				{
					break;
				}
			}
		}
		
		//
		// Store the 6 bits from each of the 4 characters as 3 bytes
		//
		// (Uses improved bounds checking suggested by Alexandre Colucci)
		//
		if(accumulateIndex >= 2)  
			outputBuffer[j] = (accumulated[0] << 2) | (accumulated[1] >> 4);  
		if(accumulateIndex >= 3)  
			outputBuffer[j + 1] = (accumulated[1] << 4) | (accumulated[2] >> 2);  
		if(accumulateIndex >= 4)  
			outputBuffer[j + 2] = (accumulated[2] << 6) | accumulated[3];
		j += accumulateIndex - 1;
	}
	
	decoded.assign( ( const char* ) outputBuffer, j );
	
	if ( outputBuffer )
	{
		free( outputBuffer );
	}
	
	return decoded;
}
