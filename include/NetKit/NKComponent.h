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
 
#ifndef _netkit_component_h
#define _netkit_component_h

#include <NetKit/NKObject.h>
#include <NetKit/NKError.h>
#include <list>

namespace netkit {

class component : public object
{
public:

	typedef smart_ptr< component >	ptr;
	typedef std::list< ptr >		list;

	inline static list::iterator
	begin()
	{
		return m_instances->begin();
	}
	
	inline static list::iterator
	end()
	{
		return m_instances->end();
	}

	virtual status
	will_initialize() = 0;
	
	virtual status
	did_initialize() = 0;
	
	virtual void
	will_terminate() = 0;
	
	inline status
	status() const
	{
		return m_status;
	}
	
protected:
	
	component();
	
	virtual
	~component();
	
	enum status m_status;
	
private:

	static list *m_instances;
};

}

#define DECLARE_COMPONENT( NAME )				\
public:											\
static NAME::ptr								\
instance();										\
virtual netkit::status							\
will_initialize();								\
virtual netkit::status							\
did_initialize();								\
virtual void									\
will_terminate();

#define DEFINE_COMPONENT1( NAME )				\
static NAME g_instance;							\
NAME::ptr										\
NAME::instance()								\
{												\
	if ( g_instance.status() != netkit::status::ok )				\
	{											\
		g_instance.will_initialize();			\
	}											\
												\
	return &g_instance;							\
}

#define DEFINE_COMPONENT2( PARENT, NAME )		\
static NAME g_instance;							\
PARENT::ptr										\
PARENT::instance()								\
{												\
	return NAME::instance();					\
}												\
NAME::ptr										\
NAME::instance()								\
{												\
	if ( g_instance.status() != netkit::status::ok )	\
	{											\
		g_instance.will_initialize();			\
	}											\
												\
	return &g_instance;							\
}

#endif
