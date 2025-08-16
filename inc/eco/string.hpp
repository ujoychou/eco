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
#include <eco/string/stream.hpp>
#include <eco/string/format.hpp>


eco_namespace(eco)
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