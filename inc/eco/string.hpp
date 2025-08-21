#pragma once
/*******************************************************************************
@ name

@ function

@ exception

@ note

--------------------------------------------------------------------------------
@ [2024-08-21] ujoy created


--------------------------------------------------------------------------------
* copyright(c) 2024 - 2027, ujoy, reserved all right.

*******************************************************************************/
#include <eco/string/string_c.hpp>
#include <eco/string/string_view.hpp>
#include <eco/string/stream.hpp>
#include <eco/string/format.hpp>


eco_namespace(eco)
////////////////////////////////////////////////////////////////////////////////
// version format: 1.2.1 < 1.11.1 < 1.12 < 1.12.0
inline int compare_version(const char* s1, const char* s2)
{
	uint32_t v1 = 0;
	uint32_t v2 = 0;
	uint32_t pos1 = find_first(s1, '.');
	uint32_t pos2 = find_first(s2, '.');
	while (pos1 != eco::U32_1 && pos2 != eco::U32_1)
	{
		v1 = eco::cast<uint32_t>(eco::string_view(s1, pos1));
		v2 = eco::cast<uint32_t>(eco::string_view(s2, pos1));
		if (v1 != v2) { break; }
		s1 += pos1 + 1;
		s2 += pos2 + 1;
		pos1 = find_first(s1, '.');
		pos2 = find_first(s2, '.');
	}
	return v1 < v2 ? -1 : (v1 > v2 ? 1 : 
		(pos1 == pos2 ? 0 : pos1 == eco::U32_1 ? -1 : 1));
}


////////////////////////////////////////////////////////////////////////////////
class string 
	: public eco::string_c
	, public eco::stream_t<eco::string>
	, public eco::format_t<eco::string>
{
public:
	inline string() {}

	explicit inline string(uint32_t size, bool_t reserved = false)
	{
		reserved ? reserve(size) : resize(size);
	}

	explicit inline string(const char* v)
	{
		eco::string_c::assign(v);
	}

	explicit inline string(const char* v, uint32_t size)
	{
		eco::string_c::assign(v, size);
	}

	explicit inline string(const std::string& v)
	{
		eco::string_c::assign(v.c_str(), static_cast<uint32_t>(v.size()));
	}

	inline string(eco::string&& v) : eco::string_c(std::move(v))
	{}

	inline string& operator=(eco::string&& v)
	{
		eco::string_c::operator=(std::move(v));
		return *this;
	}

	inline string& operator=(const char* v)
	{
		eco::string_c::assign(v);
		return *this;
	}

	inline string& operator=(const std::string& v)
	{
		eco::string_c::assign(v);
		return *this;
	}

	inline string& operator=(const eco::string& v)
	{
		eco::string_c::assign(v);
		return *this;
	}

	inline bool operator==(const char* v) const
	{
		return m_data ? eco::equal(m_data, v) : false;
	}

	inline bool operator==(const std::string& v) const
	{
		return operator==(v.c_str());
	}

	inline bool operator==(const eco::string& v) const
	{
		return operator==(v.c_str());
	}
};


////////////////////////////////////////////////////////////////////////////////
template<typename buffer_t>
class input
	: public eco::stream_t<eco::input<buffer_t>>
	, public eco::format_t<eco::input<buffer_t>>
{
public:
	inline input(buffer_t& buffer) : buff(buffer) 
	{}

	inline input& append(char c, uint32_t size)
	{
		buff.append(c, size);
		return *this;
	}

	inline input& append(const char* str, uint32_t size)
	{
		buff.append(str, size);
		return *this;
	}

	buffer_t& buff;
};


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(eco)