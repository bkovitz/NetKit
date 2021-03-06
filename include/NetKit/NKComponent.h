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
#include <vector>
#include <list>

namespace netkit {

class NETKIT_DLL component : public object
{
public:

	typedef smart_ref< component >	ref;
	typedef std::list< ref >		list;
	
	static bool
	initialize( const std::vector< std::string > &command_line );
	
	static void
	finalize();

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

	virtual netkit::status
	will_initialize( const std::vector< std::string > &command_line ) = 0;
	
	virtual netkit::status
	did_initialize() = 0;
	
	virtual void
	will_terminate() = 0;
	
	inline netkit::status
	status() const
	{
		return m_status;
	}
	
protected:
	
	component();
	
	virtual
	~component();
	
	netkit::status m_status;
	
private:

	friend void netkit::initialize();

	static list *m_instances;
};

}

#define DECLARE_COMPONENT( NAME )									\
public:																\
static NAME::ref													\
instance();															\
virtual netkit::status												\
will_initialize( const std::vector< std::string > &command_line );	\
virtual netkit::status												\
did_initialize();													\
virtual void														\
will_terminate();

#define DEFINE_COMPONENT1( NAME )				\
static NAME g_instance;							\
NAME::ref										\
NAME::instance()								\
{												\
	return &g_instance;							\
}

#define DEFINE_COMPONENT2( PARENT, NAME )		\
static NAME g_instance;							\
PARENT::ref										\
PARENT::instance()								\
{												\
	return NAME::instance();					\
}												\
NAME::ref										\
NAME::instance()								\
{												\
	return &g_instance;							\
}

#endif
