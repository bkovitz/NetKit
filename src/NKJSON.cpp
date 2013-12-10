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
/*
 * Copyright (c) 2011 Anhero Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <NetKit/NKJSON.h>
#include <NetKit/NKOutputFilter.h>
#include <NetKit/NKUnicode.h>
#include <NetKit/NKPlatform.h>
#include <NetKit/NKLog.h>
#include <cassert>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <string>
#include <stack>
#include <list>

using namespace netkit;
using namespace netkit::json;

namespace structural
{
	const char BEGIN_ARRAY = '[';
	const char BEGIN_OBJECT = '{';
	const char END_ARRAY = ']';
	const char END_OBJECT = '}';
	const char NAME_SEPARATOR = ':';
	const char VALUE_SEPARATOR = ',';
	const char BEGIN_END_STRING = '"';
}
	
namespace whitespace
{
	const char SPACE = ' ';
	const char HORIZONTAL_TAB = '\t';
	const char NEW_LINE = '\n';
	const char CARRIAGE_RETURN = '\r';
}
	
namespace literals
{
	const std::string FALSE_STRING = "false";
	const std::string TRUE_STRING = "true";
	const std::string NULL_STRING = "null";
}
	
namespace numbers
{
	const std::string DIGITS = "0123456789ABCDEFabcdef";
	const char DECIMAL_POINT = '.';
	const char LOWER_EXP = 'e';
	const char UPPER_EXP = 'E';
	const char MINUS = '-';
	const char PLUS = '+';
}
	
namespace strings
{
	namespace std
	{
		const char QUOTATION_MARK = '"';
		const char REVERSE_SOLIDUS = '\\';
		const char SOLIDUS = '/';
		const char BACKSPACE = '\b';
		const char FORM_FEED = '\f';
		const char LINE_FEED = '\n';
		const char CARRIAGE_RETURN = '\r';
		const char TAB = '\t';
	}
	
	namespace Json
	{
		const ::std::string QUOTATION_MARK = "\\\"";
		const ::std::string REVERSE_SOLIDUS = "\\\\";
		const ::std::string SOLIDUS = "\\/";
		const ::std::string BACKSPACE = "\\b";
		const ::std::string FORM_FEED = "\\f";
		const ::std::string LINE_FEED = "\\n";
		const ::std::string CARRIAGE_RETURN = "\\r";
		const ::std::string TAB = "\\t";
		const ::std::string BEGIN_UNICODE = "\\u";
		
		namespace escape
		{
			const char BEGIN_ESCAPE = '\\';
			const char QUOTATION_MARK = '"';
			const char REVERSE_SOLIDUS = '\\';
			const char SOLIDUS = '/';
			const char BACKSPACE = 'b';
			const char FORM_FEED = 'f';
			const char LINE_FEED = 'n';
			const char CARRIAGE_RETURN = 'r';
			const char TAB = 't';
			const char BEGIN_UNICODE = 'u';
		}
	}
}


class Indenter
{
public:

	Indenter()
	:
		atStartOfLine(true)
	{
	}

	std::streambuf::int_type operator()(std::streambuf &destination, std::streambuf::int_type character)
	{
		std::streambuf::char_type tmpChar = std::streambuf::traits_type::to_char_type(character);

		if (atStartOfLine && tmpChar != whitespace::NEW_LINE)
		{
			destination.sputc(whitespace::HORIZONTAL_TAB);
		}

		atStartOfLine = (tmpChar == whitespace::NEW_LINE);
		return destination.sputc(tmpChar);
	}

private:

	bool atStartOfLine;
};

class IndentCanceller
{
public:

	IndentCanceller()
	:
		afterBackSlash(false),
		inString(false)
	{
	}

	std::streambuf::int_type operator()(std::streambuf &destination, std::streambuf::int_type character)
	{
		std::streambuf::char_type tmpChar = std::streambuf::traits_type::to_char_type(character);

		// If we encounter a quotation mark.
		if (tmpChar == structural::BEGIN_END_STRING) {
			// If we're not in a string, we change that. If we're in a string,
			// we change that only if we're not after an escape back slash.
			inString = !inString || (afterBackSlash);
		}

		// We determine if we start a backslash escape or not.
		afterBackSlash = inString && !afterBackSlash && (tmpChar == strings::Json::escape::BEGIN_ESCAPE);

		return (tmpChar != whitespace::NEW_LINE && tmpChar != whitespace::HORIZONTAL_TAB && tmpChar != whitespace::CARRIAGE_RETURN && (inString || tmpChar != whitespace::SPACE)) ? (destination.sputc(tmpChar)) : (0);
	}
	
private:

	bool afterBackSlash;
	bool inString;
};


class escaper
{
public:

	escaper()
	:
		afterBackSlash(false),
		inString(false)
	{
	}

	std::streambuf::int_type operator()(std::streambuf &destination, std::streambuf::int_type character)
	{
		bool notescaped = true;
		std::streambuf::char_type tmpChar = std::streambuf::traits_type::to_char_type(character);

		// If we encounter a quotation mark.
		if (tmpChar == structural::BEGIN_END_STRING) {
			// If we're not in a string, we change that. If we're in a string,
			// we change that only if we're not after an escape back slash.
			inString = !inString || (afterBackSlash);

		}
		else if (inString && !afterBackSlash)
		{
			// If we are in a string definition and we're not after a backslash
			// escape.
			if (tmpChar == strings::std::REVERSE_SOLIDUS) {
				destination.sputn(strings::Json::REVERSE_SOLIDUS.c_str(), strings::Json::REVERSE_SOLIDUS.size());
				notescaped = false;

			} else if (tmpChar == strings::std::BACKSPACE) {
				destination.sputn(strings::Json::BACKSPACE.c_str(), strings::Json::BACKSPACE.size());
				notescaped = false;

			} else if (tmpChar == strings::std::FORM_FEED) {
				destination.sputn(strings::Json::FORM_FEED.c_str(), strings::Json::FORM_FEED.size());
				notescaped = false;

			} else if (tmpChar == strings::std::LINE_FEED) {
				destination.sputn(strings::Json::LINE_FEED.c_str(), strings::Json::LINE_FEED.size());
				notescaped = false;

			} else if (tmpChar == strings::std::TAB) {
				destination.sputn(strings::Json::TAB.c_str(), strings::Json::TAB.size());
				notescaped = false;

			} else if (tmpChar >= '\0' && tmpChar <= '\x1f') {
				std::string tmp(value::escape_to_unicode(tmpChar));
				destination.sputn(tmp.c_str(), tmp.size());
				notescaped = false;
			}

		}

		// We determine if we start a backslash escape or not.
		afterBackSlash = inString && !afterBackSlash && (tmpChar == strings::Json::escape::BEGIN_ESCAPE);
		return (notescaped) ? (destination.sputc(tmpChar)) : (0);
	}
	
private:

	bool afterBackSlash;
	bool inString;
};


class Solidusescaper
{
public:

	Solidusescaper()
	:
		afterBackSlash(false),
		inString(false)
	{
	}

	std::streambuf::int_type operator()(std::streambuf &destination, std::streambuf::int_type character)
	{
		bool notescaped = true;
		std::streambuf::char_type tmpChar = std::streambuf::traits_type::to_char_type(character);

		// If we encounter a quotation mark.
		if (tmpChar == strings::Json::escape::QUOTATION_MARK) {
			// If we're not in a string, we change that. If we're in a string,
			// we change that only if we're not after an escape back slash.
			inString = !inString || (afterBackSlash);

		} else if (inString && !afterBackSlash) {
			// If we are in a string definition and we're not after a backslash
			// escape.
			if (tmpChar == strings::std::SOLIDUS) {
				destination.sputn(strings::Json::SOLIDUS.c_str(), strings::Json::SOLIDUS.size());
				notescaped = false;

			}

		}

		// We determine if we start a backslash escape or not.
		afterBackSlash = inString && !afterBackSlash && (tmpChar == strings::Json::escape::BEGIN_ESCAPE);
		return (notescaped) ? (destination.sputc(tmpChar)) : (0);
	}
	
private:

	bool afterBackSlash;
	bool inString;
};


class array_map
{
public:

	typedef std::vector< value::ref > container;
	typedef container::value_type value_type;
	typedef container::allocator_type allocator_type;
	typedef container::size_type size_type;
	typedef container::difference_type difference_type;
	typedef container::reference reference;
	typedef container::const_reference const_reference;
	typedef container::pointer pointer;
	typedef container::const_pointer const_pointer;
	typedef container::iterator iterator;
	typedef container::const_iterator const_iterator;
	typedef container::reverse_iterator reverse_iterator;
	typedef container::const_reverse_iterator const_reverse_iterator;

	array_map( const allocator_type &alloc = allocator_type() );

	explicit array_map( size_type count, const_reference value = value_type(), const allocator_type &alloc = allocator_type() );

	template <typename InputIterator>
	array_map( InputIterator first, InputIterator last, const allocator_type &alloc = allocator_type() )
	:
		m_data( first, last )
	{
	}

	array_map(const array_map &other);

	array_map&
	operator=(const array_map &other);

	bool
	operator==(const array_map &rhs) const;

	bool
	operator!=(const array_map &rhs) const;

	bool
	operator<(const array_map &rhs) const;

	bool
	operator<=(const array_map &rhs) const;

	bool
	operator>(const array_map &rhs) const;

	bool
	operator>=(const array_map &rhs) const;

	void
	assign(size_type count, const_reference value);

	template <typename InputIterator>
	void
	assign(InputIterator first, InputIterator last)
	{
		m_data.assign(first, last);
	}

	allocator_type
	get_allocator() const;

	reference
	at(size_type pos);

	const_reference
	at(size_type pos) const;

	reference
	operator[](size_type pos);

	const_reference
	operator[](size_type pos) const;

	reference
	front();

	const_reference
	front() const;

	reference back();

	const_reference
	back() const;

	iterator
	begin();

	const_iterator
	begin() const;

	iterator
	end();

	const_iterator
	end() const;

	reverse_iterator
	rbegin();

	const_reverse_iterator
	rbegin() const;

	reverse_iterator
	rend();

	const_reverse_iterator
	rend() const;

	bool
	empty() const;

	size_type
	size() const;

	size_type
	max_size() const;

	void
	reserve(size_type size);

	size_type
	capacity() const;

	void
	clear();

	iterator
	insert(iterator pos, const_reference value);

	void
	insert(iterator pos, size_type count, const_reference value);

	template <typename InputIterator>
	void
	insert(iterator pos, InputIterator first, InputIterator last)
	{
		m_data.insert(pos, first, last);
	}

	iterator
	erase(iterator pos);

	iterator
	erase(iterator first, iterator last);

	void
	push_back(const_reference value);

	void
	pop_back();

	void
	resize(size_type count, const_reference value = value_type());

	void swap(array_map &other);

private:

	container m_data;
};


class object_map
{
public:

	typedef std::map<std::string, value::ref> container;
	typedef container::key_type key_type;
	typedef container::mapped_type mapped_type;
	typedef container::value_type value_type;
	typedef container::size_type size_type;
	typedef container::difference_type difference_type;
	typedef container::key_compare key_compare;
	typedef container::allocator_type allocator_type;
	typedef container::reference reference;
	typedef container::const_reference const_reference;
	typedef container::pointer pointer;
	typedef container::const_pointer const_pointer;
	typedef container::iterator iterator;
	typedef container::const_iterator const_iterator;
	typedef container::reverse_iterator reverse_iterator;
	typedef container::const_reverse_iterator const_reverse_iterator;

	explicit object_map(const key_compare &comp = key_compare(), const allocator_type &alloc = allocator_type());

	template <typename InputIterator>
	explicit object_map(InputIterator first, InputIterator last, const key_compare &comp = key_compare(), const allocator_type &alloc = allocator_type()) : m_data(first, last, comp, alloc)
	{
	}

	object_map(const object_map &other);

	object_map &operator=(const object_map &other);
	
	bool operator==(const object_map &rhs) const;
	
	bool operator!=(const object_map &rhs) const;
	
	bool operator<(const object_map &rhs) const;
	
	bool operator<=(const object_map &rhs) const;
	
	bool operator>(const object_map &rhs) const;
	
	bool operator>=(const object_map &rhs) const;

	allocator_type get_allocator() const;

	mapped_type&
	operator[](const key_type &key);

	iterator begin();

	const_iterator begin() const;

	iterator end();

	const_iterator end() const;

	reverse_iterator rbegin();

	const_reverse_iterator rbegin() const;

	reverse_iterator rend();

	const_reverse_iterator rend() const;

	bool empty() const;

	size_type size() const;

	size_type max_size() const;

	void clear();

	std::pair<iterator, bool> insert(const_reference value);

	iterator insert(iterator hint, const_reference value);

	template <typename InputIterator>
	void insert(InputIterator first, InputIterator last)
	{
		m_data.insert(first, last);
	}

	void erase(iterator position);

	void erase(iterator first, iterator last);

	size_type erase(const key_type &key);

	void swap(object_map &other);

	size_type count(const key_type &key) const;

	iterator find(const key_type &key);

	const_iterator find(const key_type &key) const;

	std::pair<iterator, iterator> equal_range(const key_type &key);

	std::pair<const_iterator, const_iterator> equal_range(const key_type &key) const;

	iterator lower_bound(const key_type &key);

	const_iterator lower_bound(const key_type &key) const;

	iterator upper_bound(const key_type &key);

	const_iterator upper_bound(const key_type &key) const;

	key_compare key_comp() const;

private:

	container m_data;
};


object_map::object_map(const key_compare &comp, const allocator_type &alloc)
:
	m_data(comp, alloc)
{
}


object_map::object_map(const object_map &other)
:
	m_data(other.m_data)
{
}


object_map&
object_map::operator=(const object_map &other)
{
	m_data = other.m_data;
	return *this;
}

	
bool
object_map::operator==(const object_map &rhs) const
{
	return ( m_data == rhs.m_data );
}

	
bool
object_map::operator!=(const object_map &rhs) const
{
	return m_data != rhs.m_data;
}


bool
object_map::operator<(const object_map &rhs) const
{
	return m_data < rhs.m_data;
}


bool
object_map::operator<=(const object_map &rhs) const
{
	return m_data <= rhs.m_data;
}

	
bool
object_map::operator>(const object_map &rhs) const
{
	return m_data > rhs.m_data;
}

	
bool
object_map::operator>=(const object_map &rhs) const
{
	return m_data >= rhs.m_data;
}


object_map::allocator_type
object_map::get_allocator() const
{
	return m_data.get_allocator();
}


object_map::mapped_type&
object_map::operator[](const key_type &key)
{
	return m_data[key];
}


object_map::iterator
object_map::begin()
{
	return m_data.begin();
}


object_map::const_iterator
object_map::begin() const
{
	return m_data.begin();
}


object_map::iterator
object_map::end()
{
	return m_data.end();
}


object_map::const_iterator
object_map::end() const
{
	return m_data.end();
}


object_map::reverse_iterator
object_map::rbegin()
{
	return m_data.rbegin();
}


object_map::const_reverse_iterator
object_map::rbegin() const
{
	return m_data.rbegin();
}


object_map::reverse_iterator
object_map::rend()
{
	return m_data.rend();
}


object_map::const_reverse_iterator
object_map::rend() const
{
	return m_data.rend();
}


bool
object_map::empty() const
{
	return m_data.empty();
}


object_map::size_type
object_map::size() const
{
	return m_data.size();
}


object_map::size_type
object_map::max_size() const
{
	return m_data.max_size();
}


void
object_map::clear()
{
	m_data.clear();
}


std::pair<object_map::iterator, bool>
object_map::insert(const_reference value)
{
	return m_data.insert(value);
}


object_map::iterator
object_map::insert(iterator hint, const_reference value)
{
	return m_data.insert(hint, value);
}


void
object_map::erase(iterator position)
{
	m_data.erase(position);
}


void
object_map::erase(iterator first, iterator last)
{
	m_data.erase(first, last);
}


object_map::size_type
object_map::erase(const key_type &key)
{
	return m_data.erase(key);
}


void
object_map::swap(object_map &other)
{
	m_data.swap(other.m_data);
}


object_map::size_type
object_map::count(const key_type &key) const
{
	return m_data.count(key);
}


object_map::iterator
object_map::find(const key_type &key)
{
	return m_data.find(key);
}


object_map::const_iterator
object_map::find(const key_type &key) const
{
	return m_data.find(key);
}


std::pair<object_map::iterator, object_map::iterator>
object_map::equal_range(const key_type &key)
{
	return m_data.equal_range(key);
}


std::pair<object_map::const_iterator, object_map::const_iterator>
object_map::equal_range(const key_type &key) const
{
	return m_data.equal_range(key);
}


object_map::iterator
object_map::lower_bound(const key_type &key)
{
	return m_data.lower_bound(key);
}


object_map::const_iterator
object_map::lower_bound(const key_type &key) const
{
	return m_data.lower_bound(key);
}


object_map::iterator
object_map::upper_bound(const key_type &key)
{
	return m_data.upper_bound(key);
}


object_map::const_iterator
object_map::upper_bound(const key_type &key) const
{
	return m_data.upper_bound(key);
}


object_map::key_compare
object_map::key_comp() const
{
	return m_data.key_comp();
}


std::ostream&
netkit::json::operator<<(std::ostream &output, const object_map &o)
{
	if (o.empty())
	{
		output << structural::BEGIN_OBJECT << structural::END_OBJECT;
	}
	else
	{
		output << structural::BEGIN_OBJECT << std::endl;
		netkit::output_filter<Indenter> indent(output.rdbuf());
		output.rdbuf(&indent);

		for (object_map::const_iterator i = o.begin(); i != o.end(); ++i)
		{
			if (i != o.begin())
			{
				output << structural::VALUE_SEPARATOR << std::endl;
			}

			output << structural::BEGIN_END_STRING << value::escape_minimum_characters(i->first) << structural::BEGIN_END_STRING << whitespace::SPACE << structural::NAME_SEPARATOR << whitespace::SPACE << i->second;
		}

		output.rdbuf(indent.destination());

		output << std::endl << structural::END_OBJECT;
	}

	return output;
}


array_map::array_map(const allocator_type &alloc)
:
	m_data(alloc)
{
}


array_map::array_map(size_type count, const_reference value, const allocator_type &alloc)
:
	m_data(count, value, alloc)
{
}


array_map::array_map(const array_map &other)
:
	m_data(other.m_data)
{
}


array_map&
array_map::operator=(const array_map &other)
{
	m_data = other.m_data;
	return *this;
}


bool
array_map::operator==(const array_map &rhs) const
{
	return m_data == rhs.m_data;
}


bool
array_map::operator!=(const array_map &rhs) const
{
	return m_data != rhs.m_data;
}


bool
array_map::operator<(const array_map &rhs) const
{
	return m_data < rhs.m_data;
}


bool
array_map::operator<=(const array_map &rhs) const
{
	return m_data <= rhs.m_data;
}


bool
array_map::operator>(const array_map &rhs) const
{
	return m_data > rhs.m_data;
}


bool
array_map::operator>=(const array_map &rhs) const
{
	return m_data >= rhs.m_data;
}


void
array_map::assign(size_type count, const_reference value)
{
	m_data.assign(count, value);
}


array_map::allocator_type
array_map::get_allocator() const
{
	return m_data.get_allocator();
}


array_map::reference
array_map::at(size_type pos)
{
	return m_data.at(pos);
}


array_map::const_reference
array_map::at(size_type pos) const
{
	return m_data.at(pos);
}


array_map::reference
array_map::operator[](size_type pos)
{
	return m_data[pos];
}


array_map::const_reference
array_map::operator[](size_type pos) const
{
	return m_data[pos];
}


array_map::reference
array_map::front()
{
	return m_data.front();
}


array_map::const_reference
array_map::front() const
{
	return m_data.front();
}


array_map::reference
array_map::back()
{
	return m_data.back();
}


array_map::const_reference
array_map::back() const
{
	return m_data.back();
}


array_map::iterator
array_map::begin()
{
	return m_data.begin();
}


array_map::const_iterator
array_map::begin() const
{
	return m_data.begin();
}


array_map::iterator
array_map::end()
{
	return m_data.end();
}


array_map::const_iterator
array_map::end() const
{
	return m_data.end();
}


array_map::reverse_iterator
array_map::rbegin()
{
	return m_data.rbegin();
}


array_map::const_reverse_iterator
array_map::rbegin() const
{
	return m_data.rbegin();
}


array_map::reverse_iterator
array_map::rend()
{
	return m_data.rend();
}


array_map::const_reverse_iterator
array_map::rend() const
{
	return m_data.rend();
}


bool
array_map::empty() const
{
	return m_data.empty();
}


array_map::size_type
array_map::size() const
{
	return m_data.size();
}


array_map::size_type
array_map::max_size() const
{
	return m_data.max_size();
}


void
array_map::reserve(size_type size)
{
	m_data.reserve(size);
}


array_map::size_type
array_map::capacity() const
{
	return m_data.capacity();
}


void
array_map::clear()
{
	m_data.clear();
}


array_map::iterator
array_map::insert(iterator pos, const_reference value)
{
	return m_data.insert(pos, value);
}


void
array_map::insert(iterator pos, size_type count, const_reference value)
{
	m_data.insert(pos, count, value);
}


array_map::iterator
array_map::erase(iterator pos)
{
	return m_data.erase(pos);
}


array_map::iterator
array_map::erase(iterator first, iterator last)
{
	return m_data.erase(first, last);
}


void
array_map::push_back(const_reference value)
{
	m_data.push_back(value);
}


void
array_map::pop_back()
{
	m_data.pop_back();
}


void
array_map::resize(size_type count, const_reference value)
{
	m_data.resize(count, value);
}


void
array_map::swap(array_map &other)
{
	m_data.swap(other.m_data);
}


std::ostream&
netkit::json::operator<<(std::ostream &output, const array_map &a)
{
	if (a.empty())
	{
		output << structural::BEGIN_ARRAY << structural::END_ARRAY;
	}
	else
	{
		output << structural::BEGIN_ARRAY << std::endl;
		netkit::output_filter<Indenter> indent(output.rdbuf());
		output.rdbuf(&indent);

		for (array_map::const_iterator i = a.begin(); i != a.end(); ++i)
		{
			if (i != a.begin())
			{
				output << structural::VALUE_SEPARATOR << std::endl;
			}

			output << *i;
		}

		output.rdbuf(indent.destination());

		output << std::endl << structural::END_ARRAY;
	}

	return output;
}


std::string
value::escape_minimum_characters(const std::string &str)
{
	std::stringstream result;

	for (std::string::const_iterator i = str.begin(); i != str.end(); ++i)
	{
		if (*i == strings::std::QUOTATION_MARK)
		{
			result << strings::Json::QUOTATION_MARK;
		}
		else if (*i == strings::std::REVERSE_SOLIDUS)
		{
			result << strings::Json::REVERSE_SOLIDUS;
		}
		else if (*i == strings::std::BACKSPACE)
		{
			result << strings::Json::BACKSPACE;
		}
		else if (*i == strings::std::FORM_FEED)
		{
			result << strings::Json::FORM_FEED;
		}
		else if (*i == strings::std::LINE_FEED)
		{
			result << strings::Json::LINE_FEED;
		}
		else if (*i == strings::std::CARRIAGE_RETURN)
		{
			result << strings::Json::CARRIAGE_RETURN;
		}
		else if (*i == strings::std::TAB)
		{
			result << strings::Json::TAB;
		}
		else if (*i >= '\0' && *i <= '\x1f')
		{
			result << value::escape_to_unicode(*i);
		}
		else
		{
			result << *i;
		}
	}

	return result.str();
}


std::string
value::escape_all_characters(const std::string &str)
{
	std::stringstream result;

	for (std::string::const_iterator i = str.begin(); i != str.end(); ++i)
	{
		if (*i == strings::std::QUOTATION_MARK)
		{
			result << strings::Json::QUOTATION_MARK;
		}
		else if (*i == strings::std::REVERSE_SOLIDUS)
		{
			result << strings::Json::REVERSE_SOLIDUS;
		}
		else if (*i == strings::std::SOLIDUS)
		{
			result << strings::Json::SOLIDUS;
		}
		else if (*i == strings::std::BACKSPACE)
		{
			result << strings::Json::BACKSPACE;
		}
		else if (*i == strings::std::FORM_FEED)
		{
			result << strings::Json::FORM_FEED;
		}
		else if (*i == strings::std::LINE_FEED)
		{
			result << strings::Json::LINE_FEED;
		}
		else if (*i == strings::std::CARRIAGE_RETURN)
		{
			result << strings::Json::CARRIAGE_RETURN;
		}
		else if (*i == strings::std::TAB)
		{
			result << strings::Json::TAB;
		}
		else if (*i >= '\0' && *i <= '\x1f')
		{
			result << value::escape_to_unicode(*i);
		}
		else
		{
			result << *i;
		}
	}

	return result.str();
}


const std::string
value::escape_to_unicode(char charToescape)
{
	std::stringstream result;

	if (charToescape >= '\0' && charToescape <= '\x1f')
	{
		result << "\\u00";
		result << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(charToescape);
	}

	return result.str();
}


value::ref
value::array()
{
	return new value( type::array );
}


value::ref
value::object()
{
	return new value( type::object );
}


const value::ref
value::null()
{
	static const value::ref v = new value;
	
	return v;
}


expected< value::ref >
value::load( const std::string &s )
{
	value::ref v = new value;
	
	v->load_from_string( s );
	
	return v;
}


value::value()
:
	m_kind(type::null)
{
	m_data.m_string = NULL;
}


value::value(std::istream &input)
:
	m_kind(type::null)
{
	load_from_stream(input);
}


value::value(const std::string &newString)
:
	m_kind( type::string ),
	m_data( new std::string(newString))
{
}


value::value(const char *newCString)
:
	m_kind( type::string ),
	m_data( new std::string(newCString))
{
}


value::value( std::int8_t val )
:
	m_kind( type::integer ),
	m_data( new std::uint64_t( val ) )
{
}


value::value( std::uint8_t val )
:
	m_kind( type::integer ),
	m_data( new std::uint64_t( val ) )
{
}


value::value( std::int16_t val )
:
	m_kind( type::integer ),
	m_data( new std::uint64_t( val ) )
{
}


value::value( std::uint16_t val )
:
	m_kind( type::integer ),
	m_data( new std::uint64_t( val ) )
{
}


value::value( std::int32_t val )
:
	m_kind( type::integer ),
	m_data( new std::uint64_t( val ) )
{
}


value::value( std::uint32_t val )
:
	m_kind( type::integer ),
	m_data( new std::uint64_t( val ) )
{
}


value::value( std::int64_t val )
:
	m_kind( type::integer ),
	m_data( new std::uint64_t( val ) )
{
}


value::value( std::uint64_t val )
:
	m_kind( type::integer ),
	m_data( new std::uint64_t( val ) )
{
}


value::value( status v )
:
	m_kind( type::integer ),
	m_data( new std::uint64_t( ( std::uint64_t ) v ) )
{
}


value::value(double newDouble)
:
	m_kind( type::real ),
	m_data(new double(newDouble))
{
}


value::value(bool newBoolean)
:
	m_kind( type::boolean ),
	m_data(new bool(newBoolean))
{
}


value::value( type t )
:
	m_kind( t )
{
	switch ( t )
	{
		case type::string:
		{
			m_data.m_string = new std::string;
		}
		break;
		
		case type::integer:
		{
			m_data.m_integer = new std::uint64_t;
		}
		break;
		
		case type::real:
		{
			m_data.m_real = new double;
		}
		break;
		
		case type::object:
		{
			m_data.m_object = new object_map;
		}
		break;
		
		case type::array:
		{
			m_data.m_array = new array_map;
		}
		break;
		
		case type::boolean:
		{
			m_data.m_bool = new bool;
		}
		break;
		
		default:
		{
			m_data.m_string = NULL;
		}
		break;
	}
}



value::value(const value &v )
{
	assign( v );
}


value::~value()
{
	clear();
}


bool
value::equal( const value &v ) const
{
	bool	result;

	if ( this == &v )
	{
		result = true;
		goto exit;
	}
	
	if ( m_kind != v.m_kind)
	{
		result = false;
		goto exit;
	}
	
	switch ( m_kind )
	{
		case type::string:
		{
			result = (*m_data.m_string == *v.m_data.m_string);
		}
		break;

		case type::integer:
		{
			result = (*m_data.m_integer == *v.m_data.m_integer);
		}
		break;

		case type::real:
		{
			result = (*m_data.m_real == *v.m_data.m_real);
		}
		break;

		case type::object:
		{
			result = (*m_data.m_object == *v.m_data.m_object);
		}
		break;

		case type::array:
		{
			result = (*m_data.m_array == *v.m_data.m_array);
		}
		break;

		case type::boolean:
		{
			result = (*m_data.m_bool == *v.m_data.m_bool);
		}
		break;
		
		case type::null:
		{
			result = true;
		}
		break;

		default:
		{
			result = false;
		}
		break;
	}
	
exit:

	return result;
}


void
value::assign( const value &v )
{
	clear();

	m_kind = v.m_kind;
	
	switch ( m_kind )
	{
		case type::string:
			m_data.m_string = new std::string( *v.m_data.m_string);
			break;

		case type::integer:
			m_data.m_integer = new std::uint64_t( *v.m_data.m_integer);
			break;

		case type::real:
			m_data.m_real = new double( *v.m_data.m_real);
			break;

		case type::object:
			m_data.m_object = new object_map( *v.m_data.m_object);
			break;

		case type::array:
			m_data.m_array = new array_map( *v.m_data.m_array);
			break;

		case type::boolean:
			m_data.m_bool = new bool( *v.m_data.m_bool);
			break;

		default:
			m_kind = type::null;
			break;
	}
}


value&
value::operator=(const value &v)
{
	if ( this != &v )
	{
		clear();
		
		assign( v );
	}
	
	return *this;
}


bool
value::operator==(const value &rhs) const
{
	return equal( rhs );
}


bool
value::operator!=(const value &rhs) const
{
	return !equal( rhs );
}


bool
value::operator<(const value &rhs) const
{
	bool result = false;

	if (this != &rhs)
	{
		if ( m_kind == rhs.m_kind)
		{
			switch ( m_kind )
			{
				case type::string:
					result = (*m_data.m_string < *m_data.m_string);
					break;

				case type::integer:
					result = (*m_data.m_integer < *m_data.m_integer);
					break;

				case type::real:
					result = (*m_data.m_real < *m_data.m_real);
					break;

				case type::object:
					result = (*m_data.m_object < *m_data.m_object);
					break;

				case type::array:
					result = (*m_data.m_array < *m_data.m_array);
					break;

				case type::boolean:
					result = (*m_data.m_bool < *m_data.m_bool);
					break;

				default:
					break;
				}

			}
		}

		return result;
	}


bool
value::operator<=(const value &rhs) const
{
	return *this < rhs || *this == rhs;
}


bool
value::operator>(const value &rhs) const
{
		bool result = false;

		if (this != &rhs) {
			if ( m_kind == rhs.m_kind) {
				switch (m_kind) {
				case type::string:
					result = (*m_data.m_string > *m_data.m_string);
					break;

				case type::integer:
					result = (*m_data.m_integer > *m_data.m_integer);
					break;

				case type::real:
					result = (*m_data.m_real > *m_data.m_real);
					break;

				case type::object:
					result = (*m_data.m_object > *m_data.m_object);
					break;

				case type::array:
					result = (*m_data.m_array > *m_data.m_array);
					break;

				case type::boolean:
					result = (*m_data.m_bool	> *m_data.m_bool);
					break;

				default:
					break;
				}

			}
		}

		return result;
	}


bool
value::operator>=(const value &rhs) const
{
	return *this > rhs || *this == rhs;
}


value::ref
value::operator[](const char *key)
{
	return operator[]( std::string( key ) );
}


value::ref
value::operator[](const object_map::key_type &key)
{
	if ( m_kind != type::object )
	{
		clear();
		m_kind = type::object;
		m_data.m_object = new object_map();
	}

	return ( *m_data.m_object )[ key ];
}


value::ref
value::at_index( std::size_t index )
{
	if (m_kind != type::array )
	{
		clear();
		m_kind = type::array;
		m_data.m_array = new array_map(index + 1);
	}

	return ( *m_data.m_array )[index];
}


value::type
value::kind() const
{
	return m_kind;
}


bool
value::is_string() const
{
	return ( m_kind == type::string );
}


bool
value::is_integer() const
{
	return m_kind == type::integer;
}


bool
value::is_status() const
{
	return m_kind == type::integer;
}


bool
value::is_real() const
{
	return m_kind == type::real;
}


bool
value::is_member( const std::string &key ) const
{
	if ( m_kind == type::object )
	{
		auto it = m_data.m_object->find( key );
		return ( it != m_data.m_object->end() );
	}
	else
	{
		return false;
	}
}


value::keys
value::all_keys() const
{
	value::keys ret;
	
	if ( is_object() )
	{
		for ( auto it = m_data.m_object->begin(); it != m_data.m_object->end(); it++ )
		{
			ret.push_back( it->first );
		}
	}
	
	return ret;
}


bool
value::is_object() const
{
	return m_kind == type::object;
}


bool
value::is_array() const
{
	return m_kind == type::array;
}


bool
value::is_bool() const
{
	return m_kind == type::boolean;
}


bool
value::is_null() const
{
	return m_kind == type::null;
}


std::string
value::as_string( const std::string &default_value ) const
{
	return ( m_kind == type::string ) ? *m_data.m_string : default_value;
}


void
value::set_string(std::string const &newString)
{
	if (m_kind == type::string)
	{
		*m_data.m_string = newString;
	}
	else
	{
		clear();
		m_kind = type::string;
		m_data.m_string = new std::string(newString);
	}
}


std::int8_t
value::as_int8( std::int8_t default_value ) const
{
	return ( m_kind == type::integer ) ? ( std::int8_t ) *m_data.m_integer : default_value;
}


std::uint8_t
value::as_uint8( std::uint8_t default_value ) const
{
	return ( m_kind == type::integer ) ? ( std::uint8_t ) *m_data.m_integer : default_value;
}


std::int16_t
value::as_int16( std::int16_t default_value ) const
{
	return ( m_kind == type::integer ) ? ( std::int16_t ) *m_data.m_integer : default_value;
}


std::uint16_t
value::as_uint16( std::uint16_t default_value ) const
{
	return ( m_kind == type::integer ) ? ( std::uint16_t ) *m_data.m_integer : default_value;
}


std::int32_t
value::as_int32( std::int32_t default_value ) const
{
	return ( m_kind == type::integer ) ? ( std::int32_t ) *m_data.m_integer : default_value;
}


std::uint32_t
value::as_uint32( std::uint32_t default_value ) const
{
	return ( m_kind == type::integer ) ? ( std::uint32_t ) *m_data.m_integer : default_value;
}


std::int64_t
value::as_int64( std::int64_t default_value ) const
{
	return ( m_kind == type::integer ) ? *m_data.m_integer : default_value;
}


std::uint64_t
value::as_uint64( std::uint64_t default_value ) const
{
	return ( m_kind == type::integer ) ? *m_data.m_integer : default_value;
}


std::time_t
value::as_time( std::time_t default_value ) const
{
	return ( m_kind == type::integer ) ? ( std::time_t ) *m_data.m_integer : default_value;
}


status
value::as_status( status default_value ) const
{
	return ( m_kind == type::integer ) ? ( status ) *m_data.m_integer : default_value;
}


void
value::set_integer( std::int64_t newInt)
{
	if (m_kind == type::integer)
	{
		*m_data.m_integer = newInt;
	}
	else
	{
		clear();
		m_kind = type::integer;
		m_data.m_integer = new std::uint64_t( newInt );
	}
}


double
value::as_real( double default_value ) const
{
	return ( m_kind == type::real ) ? *m_data.m_real : default_value;
}


void
value::set_real(double newDouble)
{
	if (m_kind == type::real)
	{
		*m_data.m_real = newDouble;
	}
	else
	{
		clear();
		m_kind = type::real;
		m_data.m_real = new double(newDouble);
	}
}


void
value::set_object(const object_map &newobject)
{
	if (m_kind == type::object)
	{
		*m_data.m_object = newobject;
	}
	else
	{
		clear();
		m_kind = type::object;
		m_data.m_object = new object_map(newobject);
	}
}


void
value::set_array(const array_map &newarray)
{
	if (m_kind == type::array)
	{
		*m_data.m_array = newarray;
	}
	else
	{
		clear();
		m_kind = type::array;
		m_data.m_array = new array_map(newarray);
	}
}


bool
value::append( const value::ref &v )
{
	if ( m_kind != type::array )
	{
		clear();
		m_kind = type::array;
		m_data.m_array = new array_map;
	}
	
	m_data.m_array->push_back( v );
	return true;
}


std::size_t
value::size() const
{
	if ( m_kind == type::array )
	{
		return m_data.m_array->size();
	}
	else if ( m_kind == type::object )
	{
		return m_data.m_object->size();
	}
	else
	{
		return 0;
	}
}


bool
value::as_bool( bool default_value ) const
{
	return ( m_kind == type::boolean ) ? *m_data.m_bool : default_value;
}


void
value::set_bool(bool newBoolean)
{
	if (m_kind == type::boolean)
	{
		*m_data.m_bool = newBoolean;
	}
	else
	{
		clear();
		m_kind = type::boolean;
		m_data.m_bool = new bool(newBoolean);
	}
}


void value::set_null()
{
	clear();
	m_kind = type::null;
	m_data.m_string = NULL;
}


void
value::load_from_string(std::string const &json)
{
	std::stringstream jsonStream(json);
	load_from_stream(jsonStream);
}


void
value::load_from_stream(std::istream &input)
{
	char currentCharacter;

	char encoding[2];
	input.get(encoding[0]);
	input.get(encoding[1]);

	if (encoding[0] != '\0' && encoding[1] != '\0')
	{
		input.putback(encoding[1]);
		input.putback(encoding[0]);

		bool noErrors = true;

		while (noErrors && input.good())
		{
			input.get(currentCharacter);

			if (input.good())
			{
				if (currentCharacter == structural::BEGIN_END_STRING)
				{
					set_string("");
					read_string(input, *m_data.m_string);
					noErrors = false;
				}
				else if (currentCharacter == structural::BEGIN_OBJECT)
				{
					set_object(object_map());
					read_object(input, *m_data.m_object);
					noErrors = false;
				}
				else if (currentCharacter == structural::BEGIN_ARRAY)
				{
					set_array(array_map());
					read_array(input, *m_data.m_array);
					noErrors = false;
				}
				else if (currentCharacter == literals::NULL_STRING[0])
				{
					if (!input.eof())
					{
						input.get(currentCharacter);

						if (currentCharacter == literals::NULL_STRING[1])
						{
							if (!input.eof())
							{
								input.get(currentCharacter);

								if (currentCharacter == literals::NULL_STRING[2])
								{
									if (!input.eof())
									{
										input.get(currentCharacter);

										if (currentCharacter == literals::NULL_STRING[3])
										{
											set_null();
											noErrors = false;
										}
										else
										{
											nklog( log::error, "invalid characters found" );
										}
									}
									else
									{
										nklog( log::error, "json input ends incorrectly" );
									}
								}
								else
								{
									nklog( log::error, "invalid characters found" );
								}
							}
							else
							{
								nklog( log::error, "json input ends incorrectly" );
							}
						}
						else
						{
							nklog( log::error, "invalid characters found" );
						}
					}
					else
					{
						nklog( log::error, "json input ends incorrectly" );
					}
				}
				else if (currentCharacter == numbers::MINUS || (currentCharacter >= numbers::DIGITS[0] && currentCharacter <= numbers::DIGITS[9]))
				{
					input.putback(currentCharacter);
					read_number(input, *this);
					noErrors = false;
				}
				else if (currentCharacter == literals::TRUE_STRING[0])
				{
					if (!input.eof())
					{
						input.get(currentCharacter);

						if (currentCharacter == literals::TRUE_STRING[1])
						{
							if (!input.eof())
							{
								input.get(currentCharacter);

								if (currentCharacter == literals::TRUE_STRING[2])
								{
									if (!input.eof())
									{
										input.get(currentCharacter);

										if (currentCharacter == literals::TRUE_STRING[3])
										{
											set_bool(true);
											noErrors = false;
										}
									}
								}
							}
						}
					}
				}
				else if (currentCharacter == literals::FALSE_STRING[0])
				{
					if (!input.eof())
					{
						input.get(currentCharacter);

						if (currentCharacter == literals::FALSE_STRING[1])
						{
							if (!input.eof())
							{
								input.get(currentCharacter);

								if (currentCharacter == literals::FALSE_STRING[2])
								{
									if (!input.eof())
									{
										input.get(currentCharacter);

										if (currentCharacter == literals::FALSE_STRING[3])
										{
											if (!input.eof())
											{
												input.get(currentCharacter);

												if (currentCharacter == literals::FALSE_STRING[4])
												{
													set_bool(false);
													noErrors = false;
												}
											}
										}
									}
								}
							}
						}
					}
				}
				else if (!is_white_space(currentCharacter))
				{
					nklog( log::error, "invalid character found" );
				}
			}
		}
	}
	else
	{
		nklog( log::error, "file is not in UTF-8 format" );
	}
}


void
value::load_from_file(const std::string &filePath)
{
	std::ifstream file;
	file.open(filePath.c_str());

	if (file.is_open())
	{
		load_from_stream(file);
		file.close();
	}
	else
	{
		nklog( log::error, "failed to open file '%s", filePath.c_str() );
	}
}


void
value::write_to_stream(std::ostream &output, bool indent, bool escapeAll) const
{
	this->output(output, indent, escapeAll);
}


void
value::write_to_file(const std::string &filePath, bool indent, bool escapeAll) const
{
	std::ofstream file;
	file.open(filePath.c_str());

	if (file.is_open())
	{
		write_to_stream(file, indent, escapeAll);
		file.close();
	}
	else
	{
		nklog( log::error, "failed to open file '%s'", filePath.c_str() );
	}
}


std::string
value::to_string( output_flags flags ) const
{
	std::ostringstream os;
	
	write_to_stream( os, ( flags == output_flags::pretty ) ? true : false, true );
	
	return os.str();
}


value::data::data()
:
	m_string(NULL)
{
}


value::data::data(std::string *newStringvalue)
:
	m_string(newStringvalue)
{
}


value::data::data( std::uint64_t *newIntvalue)
:
	m_integer(newIntvalue)
{
}


value::data::data(double *newDoublevalue)
:
	m_real(newDoublevalue)
{
}


value::data::data(object_map *newm_object)
:
	m_object(newm_object)
{
}


value::data::data(array_map *newm_array)
:
	m_array(newm_array)
{
}


value::data::data(bool *newBoolvalue)
:
	m_bool(newBoolvalue)
{
}


bool
value::is_hex_digit(char digit)
{
	return (digit >= numbers::DIGITS[0] && digit <= numbers::DIGITS[9]) || (digit >= numbers::DIGITS[10] && digit <= numbers::DIGITS[15]) || (digit >= numbers::DIGITS[16] && digit <= numbers::DIGITS[21]);
}


bool
value::is_white_space(char whiteSpace)
{
	return whiteSpace == whitespace::SPACE || whiteSpace == whitespace::HORIZONTAL_TAB || whiteSpace == whitespace::NEW_LINE || whiteSpace == whitespace::CARRIAGE_RETURN;
}


void
value::read_string(std::istream &input, std::string &result)
{
	bool noErrors = true, noUnicodeError = true;
	char currentCharacter, tmpCharacter;
	std::stringstream constructing;
	std::string tmpStr(4, ' ');
	std::stringstream tmpSs;
	int32_t tmpInt;
	std::wstring tmpStr32;
	unsigned int tmpCounter;

	while (noErrors && !input.eof())
	{
		input.get(currentCharacter);

		if (input.good())
		{
			if (currentCharacter & 0x80)
			{ // 0x80 --> 10000000
				// The character is part of an utf8 character.
				constructing << currentCharacter;
			}
			else if (currentCharacter == strings::Json::escape::BEGIN_ESCAPE)
			{
				if (!input.eof())
				{
					input.get(tmpCharacter);

					switch (tmpCharacter)
					{
						case strings::Json::escape::QUOTATION_MARK:
							constructing << strings::std::QUOTATION_MARK;
							break;

						case strings::Json::escape::REVERSE_SOLIDUS:
							constructing << strings::std::REVERSE_SOLIDUS;
							break;

						case strings::Json::escape::SOLIDUS:
							constructing << strings::std::SOLIDUS;
							break;

						case strings::Json::escape::BACKSPACE:
							constructing << strings::std::BACKSPACE;
							break;

						case strings::Json::escape::FORM_FEED:
							constructing << strings::std::FORM_FEED;
							break;

						case strings::Json::escape::LINE_FEED:
							constructing << strings::std::LINE_FEED;
							break;

						case strings::Json::escape::CARRIAGE_RETURN:
							constructing << strings::std::CARRIAGE_RETURN;
							break;

						case strings::Json::escape::TAB:
							constructing << strings::std::TAB;
							break;

						case strings::Json::escape::BEGIN_UNICODE:
							// TODO: Check for utf16 surrogate pairs.
							tmpCounter = 0;
							tmpStr.clear();
							tmpStr = "    ";
							noUnicodeError = true;

							while (tmpCounter < 4 && !input.eof())
							{
								input.get(tmpCharacter);

								if (is_hex_digit(tmpCharacter))
								{
									tmpStr[tmpCounter] = tmpCharacter;

								}
								else
								{
									noUnicodeError = false;
									nklog( log::warning, "invalid \\u character...skipping" );
								}

								++tmpCounter;
							}

							if (noUnicodeError)
							{
								tmpSs.str("");
								tmpSs << std::hex << tmpStr;
								tmpSs >> tmpInt;
								tmpStr32.clear();
								tmpStr32.push_back(tmpInt);
								tmpStr = narrow(tmpStr32);
								constructing << tmpStr;
							}

							break;

						default:
							break;
					}
				}
			}
			else if (currentCharacter == '"')
			{
				result = constructing.str();
				noErrors = false;
			}
			else
			{
				constructing << currentCharacter;
			}
		}
	}
}


void
value::read_object(std::istream &input, object_map &result)
{
	bool noErrors = true;
	char currentCharacter;
	std::string tmpString;

	while (noErrors && !input.eof())
	{
		input.get(currentCharacter);

		if (input.good())
		{
			if (currentCharacter == structural::BEGIN_END_STRING)
			{
				read_string(input, tmpString);
				currentCharacter = input.peek();
				read_to_non_white_space(input, currentCharacter);

				if (!input.eof())
				{
					if (currentCharacter == structural::NAME_SEPARATOR)
					{
						read_to_non_white_space(input, currentCharacter);

						if (!input.eof())
						{
							input.putback(currentCharacter);
							result[tmpString]->load_from_stream(input);

							while (!input.eof() && currentCharacter != structural::VALUE_SEPARATOR && currentCharacter != structural::END_OBJECT)
							{
								input.get(currentCharacter);
							}

							if (currentCharacter == structural::END_OBJECT)
							{
								noErrors = false;
							}
						}
					}
				}
			}
			else if (currentCharacter == structural::END_OBJECT)
			{
				noErrors = false;
			}
			else if (!is_white_space(currentCharacter))
			{
				nklog( log::warning, "expected '\"\', got %c. ignoring...", currentCharacter );
			}
		}
	}
}


void
value::read_array(std::istream &input, array_map &result)
{
	bool notDone = true;
	char currentChar;

	while (notDone && !input.eof())
	{
		input.get(currentChar);

		if (input.good())
		{
			if (currentChar == structural::END_ARRAY)
			{
				notDone = false;
			}
			else if (!is_white_space(currentChar))
			{
				input.putback(currentChar);
				result.push_back(new value);
				result.back()->m_kind = type::unknown;
				result.back()->load_from_stream(input);

				if (result.back()->m_kind == type::unknown)
				{
					result.pop_back();
				}

				while (!input.eof() && currentChar != ',' && currentChar != structural::END_ARRAY)
				{
					input.get(currentChar);
				}

				if (currentChar == structural::END_ARRAY)
				{
					notDone = false;
				}
			}
		}
	}
}


void
value::read_number(std::istream &input, value &result)
{
	bool notDone = true, inFraction = false, inExponent = false;
	char currentCharacter;
	std::stringstream constructing;

	if (!input.eof() && input.peek() == numbers::DIGITS[0])
	{
		input.get(currentCharacter);

		if (input.peek() == '0')
		{
			notDone = false;
		}
		else
		{
			input.putback(currentCharacter);
		}
	}

	while (notDone && !input.eof())
	{
		input.get(currentCharacter);

		if (currentCharacter == '-')
		{
			if (constructing.str().empty())
			{
				constructing << currentCharacter;
			}
			else
			{
				nklog( log::warning, "expected [0-9][.][e][E], got %c. ignoring...", currentCharacter );
			}
		}
		else if (currentCharacter >= '0' && currentCharacter <= '9')
		{
			constructing << currentCharacter;
		}
		else if (currentCharacter == '.')
		{
			if (!inFraction && !inExponent)
			{
				inFraction = true;
				constructing << currentCharacter;
			}
		}
		else if (currentCharacter == 'e' || currentCharacter == 'E')
		{
			if (!inExponent)
			{
				inExponent = true;
				constructing << currentCharacter;

				if (!input.eof() && (input.peek() == '-' || input.peek() == '+'))
				{
					input.get(currentCharacter);
					constructing << currentCharacter;
				}
			}
		}
		else
		{
			input.putback(currentCharacter);
			notDone = false;
		}
	}

	if (inFraction || inExponent)
	{
		double doubleResult;
		constructing >> doubleResult;
		result.set_real(doubleResult);
	}
	else
	{
		std::uint64_t intResult;
		constructing >> intResult;
		result.set_integer(intResult);
	}
}


void
value::read_to_non_white_space(std::istream &input, char &currentCharacter)
{
	do
	{
		input.get(currentCharacter);
	}
	while (!input.eof() && is_white_space(currentCharacter));
}


void
value::clear()
{
	switch ( m_kind )
	{
		case type::string:
		{
			delete m_data.m_string;
		}
		break;

		case type::integer:
		{
			delete m_data.m_integer;
		}
		break;

		case type::real:
		{
			delete m_data.m_real;
		}
		break;

		case type::object:
		{
			delete m_data.m_object;
		}
		break;

		case type::array:
		{
			delete m_data.m_array;
		}
		break;

		case type::boolean:
		{
			delete m_data.m_bool;
		}
		break;

		default:
		{
		}
		break;
	}
	
	m_data.m_string = NULL;
}


void
value::output(std::ostream &output, bool indent, bool escapeAll) const
{
	if (indent)
	{
		if (escapeAll)
		{
			output_filter<Solidusescaper> solidusescaper(output.rdbuf());
			output.rdbuf(&solidusescaper);
			output << *this;
			output.rdbuf(solidusescaper.destination());
		}
		else
		{
			output << *this;
		}
	}
	else
	{
		output_filter<IndentCanceller> indentCanceller(output.rdbuf());
		output.rdbuf(&indentCanceller);

		if (escapeAll)
		{
			output_filter<Solidusescaper> solidusescaper(output.rdbuf());
			output.rdbuf(&solidusescaper);
			output << *this;
			output.rdbuf(solidusescaper.destination());
		}
		else
		{
			output << *this;
		}

		output.rdbuf(indentCanceller.destination());
	}
}


std::ostream&
netkit::operator<<(std::ostream &output, const json::value::ref &v)
{
	return output << *v.get();
}


std::ostream&
netkit::json::operator<<(std::ostream &output, const value &v)
{
	switch ( v.kind() )
	{
		case value::type::string:
			output << structural::BEGIN_END_STRING << value::escape_minimum_characters( v.as_string() ) << structural::BEGIN_END_STRING;
			break;

		case value::type::integer:
			output << v.as_uint64();
			break;

		case value::type::real:
			output << v.as_real();
			break;

		case value::type::object:
			output << *v.m_data.m_object;
			break;

		case value::type::array:
			output << *v.m_data.m_array;
			break;

		case value::type::boolean:
			output << (v.as_bool() ? literals::TRUE_STRING: literals::FALSE_STRING);
			break;

		case value::type::null:
			output << literals::NULL_STRING;
			break;

		default:
			break;
	}

	return output;
}


#if defined( __APPLE__ )
#	pragma mark connection implementation
#endif

std::atomic< std::int32_t >	connection::m_id( 1 );

connection::connection()
{
	init();
}


connection::~connection()
{
	nklog( log::verbose, "" );
}


void
connection::init()
{
	on_close( [=]()
	{
		value::ref reply;
		value::ref error;
	
		error[ "code" ]		= status::internal_error;
		error[ "message" ]	= "Lost connection";
		reply[ "error" ]	= error;

		for ( auto it = m_reply_handlers.begin(); it != m_reply_handlers.end(); it++ )
		{
			it->second( reply );
		}
	} );
}


bool
connection::process( const std::uint8_t *buf, std::size_t len )
{
	value::ref		root;
	value::ref		error;
	std::string		msg;
	bool			ok = true;

	msg.assign( buf, buf + len );

	auto ret = value::load( msg.c_str() );
		
	if ( !ret.is_valid() )
	{
		nklog( log::error, "unable to parse %s", msg.c_str() );
		ok = false;
		goto exit;
	}
		
	nklog( log::verbose, "msg = %s", msg.c_str() );
	root = ret.get();
		
	if ( !root[ "method" ]->is_null() )
	{
		auto id = root[ "id" ];
				
		if ( validate( root, error ) )
		{
			if ( id->is_null() )
			{
				server::set_active_connection( this );
					
				server::route_notification( root );
					
				server::set_active_connection( nullptr );
			}
			else
			{
				server::set_active_connection( this );
					
				server::route_request( root, [=]( value::ref reply, bool close ) mutable
				{
					server::set_active_connection( nullptr );
	
					reply[ "jsonrpc" ]	= "2.0";
					reply[ "id" ]		= id;
							
					send( reply );
								
					if ( close )
					{
						shutdown();
					}
				} );
			}
		}
		else if ( !id->is_null() )
		{
			error[ "id" ] = id;
				
			send( error );
		}
	}
	else
	{
		auto it = m_reply_handlers.find( root[ "id" ]->as_uint64() );
				
		if ( it != m_reply_handlers.end() )
		{
			it->second( root );
	
			m_reply_handlers.erase( it );
		}
	}

exit:

	return ok;
}


bool
connection::send_notification( value::ref request )
{
	request[ "jsonrpc" ] = "2.0";
	
	return send( request );
}


bool
connection::send_request( value::ref request, reply_f reply )
{
	int64_t id = ++m_id;
	bool	ok = false;
	
	request[ "jsonrpc" ]	= "2.0";
	request[ "id" ]			= id;
	
	if ( !send( request ) )
	{
		nklog( log::error, "unable to send request" );
		goto exit;
	}
	
	m_reply_handlers[ id ] = reply;
	ok = true;
	
exit:
	
	return ok;
}


bool
connection::send( value::ref request )
{
	std::string msg;
	
	msg = request->to_string();

	sink::send( ( const std::uint8_t* ) msg.c_str(), msg.size(), [=]( int status )
	{
	} );

	return true;
}


bool
connection::validate( const value::ref &root, value::ref error)
{
	value::ref	err;
	bool		ok = true;
      
	if ( !root->is_object() || !root->is_member( "jsonrpc" ) || ( root[ "jsonrpc" ] != value::ref( "2.0" ) ) )
	{
        err[ "code" ]		= status::invalid;
        err[ "message" ]	= "Invalid JSON-RPC request.";
		
		error[ "id" ]		= value::null();
		error[ "jsonrpc" ]	= "2.0";
		error[ "error" ]	= err;
		
		ok = false;
	}
	else if ( root->is_member( "id" ) && ( root[ "id" ]->is_array() || root[ "id" ]->is_object() ) )
	{
        err[ "code" ]		= status::invalid;
        err[ "message" ]	= "Invalid JSON-RPC request.";
		
		error[ "id" ]		= value::null();
		error[ "jsonrpc" ]	= "2.0";
		error[ "error" ]	= err;
		
		ok = false;
	}
	else if ( !root->is_member( "method" ) || !root["method"]->is_string() )
	{
        err[ "code" ]		= status::invalid;
        err[ "message" ]	= "Invalid JSON-RPC request.";
		
		error[ "id" ]		= value::null();
		error["jsonrpc" ]	= "2.0";
		error[ "error" ]	= err;
		
		ok = false;
	}

	return ok;
}


void
connection::shutdown()
{
	m_source->close();

	// Don't do anything after this, as we are
	// most likely deleted
}

#if defined( __APPLE__ )
#	pragma mark server implementation
#endif

server::preflight_f				server::m_preflight_handler = []( json::value::ref request ){ return netkit::status::ok; };
server::notification_handlers	*server::m_notification_handlers;
server::request_handlers		*server::m_request_handlers;
connection::ref					server::m_active_connection;
connection::list				*server::m_connections;

void
server::adopt( connection::ref connection )
{
	if ( !m_connections )
	{
		m_connections = new connection::list;
	}

	m_connections->push_back( connection );

	connection->on_close( [=]()
	{
		auto it = std::find_if( m_connections->begin(), m_connections->end(), [=]( connection::ref inserted )
		{
			return ( inserted.get() == connection.get() );
		} );

		assert( it != m_connections->end() );
				
		m_connections->erase( it );
	} );
}


void
server::preflight( preflight_f func )
{
	m_preflight_handler = func;
}


void
server::bind( const std::string &method, std::size_t num_params, notification_f func )
{
	( *m_notification_handlers )[ method ] = std::make_pair( num_params, func );
}


void
server::bind( const std::string &method, std::size_t num_params, request_f func )
{
	( *m_request_handlers )[ method ] = std::make_pair( num_params, func );
}


void
server::route_notification( const value::ref &notification )
{
	auto it = m_notification_handlers->find( notification[ "method" ]->as_string() );
				
	if ( it != m_notification_handlers->end() )
	{
		auto params = notification[ "params" ];
						
		if ( it->second.first == params->size() )
		{
			it->second.second( params );
		}
	}
}


void
server::route_request( const value::ref &request, reply_f r )
{
	netkit::status status = m_preflight_handler( request );
	
	if ( status == netkit::status::ok )
	{
		auto it = m_request_handlers->find( request[ "method" ]->as_string() );
				
		if ( it != m_request_handlers->end() )
		{
			auto params = request[ "params" ];
							
			if ( it->second.first == params->size() )
			{
				it->second.second( params, r );
			}
			else
			{
				value::ref reply;
				value::ref error;

				nklog( log::error, "incorrect parameter count for '%s'", request[ "method" ]->as_string() );
							
				error[ "code" ]			= netkit::status::bad_params;
				error[ "message" ]		= "Invalid Paramaters.";
				reply[ "error" ]		= error;
					
				r( reply, false );
			}
		}
		else
		{
			value::ref reply;
			value::ref error;
							
			error[ "code" ]		= netkit::status::not_found;
			error[ "message" ]	= "Method not found.";
			reply[ "error" ]	= error;
				
			r( reply, false );
		}
	}
	else
	{
		value::ref reply;
		value::ref error;
							
		error[ "code" ]		= status;
		error[ "message" ]	= status_to_string( status );
		reply[ "error" ]	= error;
				
		r( reply, false );
	}
}


void
server::reply_with_error( reply_f r, netkit::status status, bool close )
{
	value::ref reply;
	value::ref error;
					
	error[ "code" ]		= status;
	error[ "message" ]	= status_to_string( status );
	reply[ "error" ]	= error;
			
	r( reply, close );
}


#if defined( __APPLE__ )
#	pragma mark client implementation
#endif

client::client()
:
	m_connection( new connection )
{
}


client::client( const connection::ref &conn )
:
	m_connection( conn )
{
}


void
client::connect( const uri::ref &uri, source::connect_reply_f reply )
{
	m_connection->connect( uri, reply );
}


bool
client::is_open() const
{
	return m_connection->is_open();
}


void
client::on_close( sink::close_f reply )
{
	m_connection->on_close( reply );
}


void
client::close()
{
	m_connection->close();
}


bool
client::send_notification( const std::string &method, value::ref params )
{
	json::value::ref request;
	
	request[ "method" ] = method;
	request[ "params" ] = params;
	
	return m_connection->send_notification( request );
}

	
bool
client::send_request( const std::string &method, value::ref params, reply_f reply )
{
	json::value::ref request;
	
	request[ "method" ] = method;
	request[ "params" ] = params;
	
	return m_connection->send_request( request, [=]( value::ref response )
	{
		netkit::status	error_code = netkit::status::ok;
		std::string		error_message;
		value::ref		result;
	
		if ( response[ "error" ]->is_null() )
		{
			result = response[ "result" ];
		}
		else
		{
			error_code		= response[ "error" ][ "code" ]->as_status();
			error_message	= response[ "error" ][ "message" ]->as_string();
		}
			
        reply( error_code, error_message, result );
	} );
}
