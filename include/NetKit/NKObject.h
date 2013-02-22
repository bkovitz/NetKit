#ifndef _netkit_object_h
#define _netkit_object_h

#include <NetKit/NKSmartPtr.h>
#include <atomic>

namespace netkit {

class object
{
public:

	typedef smart_ptr< object > ptr;

	inline void
	retain()
	{
		m_refs++;
	}
	
	inline int
	release()
	{
		int refs = --m_refs;
	
		if ( refs == 0 )
		{
			delete this;
		}
		
		return refs;
	}
	
	inline int
	refs()
	{
		return m_refs.fetch_add( 0 );
	}
	
protected:

	object()
	:
		m_refs( 0 )
	{
	}

	virtual ~object() = 0;

	typedef std::atomic< int >	atomic_int_t;
	mutable atomic_int_t		m_refs;
};

}

#endif