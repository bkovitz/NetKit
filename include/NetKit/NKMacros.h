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
 
#ifndef	_netkit_macros_h
#define	_netkit_macros_h

#include <NetKit/NKLog.h>

#define netkit_translate_errno( TEST, ERRNO, ALTERNATE_ERROR )		( ( TEST ) ? 0 : ( ERRNO ) ? ( ERRNO ) : ( ALTERNATE_ERROR ) )

#if defined( DEBUG )
#	define	netkit_check( X )							\
	do 													\
	{													\
		if ( !( X ) ) 									\
		{												\
			nklog( log::error( #X );					\
		}												\
	}													\
	while( 0 )									
#else
#	define	netkit_check( X )
#endif


#define	netkit_require( X, LABEL )						\
do 														\
{														\
	if ( !( X ) ) 										\
	{													\
		nklog( log::error, #X ); 						\
		goto LABEL;										\
	}													\
}														\
while( 0 )


#define	netkit_require_quiet( X, LABEL )				\
do 														\
{														\
	if ( !( X ) ) 										\
	{													\
		goto LABEL;										\
	}													\
}														\
while( 0 )


#define	netkit_require_action( X, ACTION, LABEL )		\
do 														\
{														\
	if ( !( X ) ) 										\
	{													\
		nklog( log::error, #X );						\
		{ ACTION; }										\
		goto LABEL;										\
	}													\
} while( 0 )

#endif