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
 
#ifndef _netkit_intrusive_list
#define _netkit_intrusive_list

#include <stddef.h>

namespace netkit {

template < class T >
class intrusive_list
{
public:

#define	GETLINK( e, o )			( *(void**)((char*) (e) + (o)) )
#define ASSIGNLINK( e, l, o )	( *((void**)((char*) (e) + (o))) = (l))

	intrusive_list()
	{
		m_head			= nullptr;
		m_tail			= nullptr;
		m_next_offset	= offsetof( T, m_next );
		m_prev_offset	= offsetof( T, m_prev );
	}
	
	~intrusive_list()
	{
	}

	inline T*
	head() const
	{
		return m_head;
	}
	
	inline T*
	tail() const
	{
		return m_tail;
	}

	inline void
	push_front( T *elem )
	{
		T *next = m_head;

		ASSIGNLINK( elem, m_head, m_next_offset );
		m_head = elem;

		if ( next )
		{
			ASSIGNLINK( next, elem, m_prev_offset );
		}
		else
		{
			m_tail = elem;
		}
		
		ASSIGNLINK( elem, nullptr, m_prev_offset );
	}
	
	inline void
	remove( T *elem)
	{
		T *next;
		T *prev;

		next = GETLINK( elem, m_next_offset );
		prev = GETLINK( elem, m_prev_offset );

		if ( prev )
		{
			ASSIGNLINK( prev, next, m_next_offset );
		}
		else
		{
			m_head = next;
		}
		
		if ( next )
		{
			ASSIGNLINK( next, prev, m_prev_offset );
		}
		else
		{
			m_tail = prev;
		}

		ASSIGNLINK( elem, nullptr, m_next_offset );
		ASSIGNLINK( elem, nullptr, m_prev_offset );
	}

private:

	T			*m_head;
	T			*m_tail;
	std::size_t	m_next_offset;
	std::size_t m_prev_offset;
};

}


#define DECLARE_INTRUSIVE_LIST_OBJECT( X )	\
X *m_next;									\
X *m_prev;

#endif