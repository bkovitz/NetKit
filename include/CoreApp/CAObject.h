#ifndef _coreapp_object_h
#define _coreapp_object_h

#include <CoreApp/CASmartPtr.h>
#include <atomic>

namespace coreapp {

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
	refs() const
	{	
		return m_refs;
	}
	
protected:

	object()
	:
		m_refs( 0 )
	{
	}

	virtual
	~object() = 0;

	typedef std::atomic< int > atomic_int_t;
	atomic_int_t m_refs;
};

}

#endif
