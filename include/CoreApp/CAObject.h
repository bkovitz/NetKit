#ifndef _CoreApp_object_h
#define _CoreApp_object_h

#include <CoreApp/smart_ptr.h>
#include <CoreApp/mutex.h>

namespace CoreApp {

class object
{
public:

	typedef smart_ptr< object > ptr;

	inline void
	retain()
	{
		synchronized( m_mutex );
		m_refs++;
	}
	
	inline int
	release()
	{
		int refs;
		
		m_mutex.lock();
		
		refs = --m_refs;

		m_mutex.unlock();
		
		if ( m_refs == 0 )
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

	mutex	m_mutex;
	int		m_refs;
};

}

#endif
