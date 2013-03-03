#ifndef _netkit_output_filter_h
#define _netkit_output_filter_h

#include <streambuf>
#include <limits>

namespace netkit {

template <typename T>
class output_filter : public std::streambuf
{
public:

	output_filter( std::streambuf *destination, T inserter, bool delete_when_finished = false )
	:
		m_destination( destination ),
		m_inserter( inserter ),
		m_delete_when_finished( delete_when_finished )
	{
	}

	output_filter( std::streambuf *destination, bool delete_when_finished = false )
	:
		m_destination( destination ),
		m_delete_when_finished( delete_when_finished )
	{
	}

	virtual ~output_filter()
	{
		if ( m_delete_when_finished && m_destination)
		{
			delete m_destination;
		}
	}

	virtual int_type
	overflow(int_type ch)
	{
		int result = std::char_traits<char_type>::eof();

		if ( ch == std::char_traits<char_type>::eof())
		{
			result = sync();
		}
		else if ( m_destination )
		{
				assert( ch >= 0 && ch <= static_cast<int_type>(std::numeric_limits<unsigned char>::max()));
				result = m_inserter( *m_destination, ch );
		}

		return result;
	}

	virtual int_type
	underflow()
	{
		return std::char_traits<char_type>::eof();
	}

	virtual int_type
	sync()
	{
		return this->std::streambuf::sync();
	}

	virtual std::streambuf*
	setbuf(char *p, int len)
	{
		return this->std::streambuf::setbuf(p, len);
	}

	T&
	inserter()
	{
		return m_inserter;
	}

	std::streambuf*
	destination() const
	{
		return m_destination;
	}

private:

	std::streambuf	*m_destination;
	T				m_inserter;
	bool			m_delete_when_finished;
};

}

#endif
