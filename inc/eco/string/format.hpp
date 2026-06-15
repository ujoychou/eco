#pragma once
/*******************************************************************************
@ name

@ function

@ exception

@ note

--------------------------------------------------------------------------------
@ [2024-11-14] ujoy created


--------------------------------------------------------------------------------
* copyright(c) 2024 - 2027, ujoy, reserved all right.

*******************************************************************************/
#include <eco/cast.hpp>
#include <eco/string/string_view.hpp>
#include <eco/string/string_c.hpp>


eco_namespace(eco)
////////////////////////////////////////////////////////////////////////////////
template<typename actual_t>
class format_t
{
public:
	inline actual_t& rthis() { return (actual_t&)(*this); }

	inline actual_t& operator%(bool v)
	{
		const char* str = eco::cast(v, eco::bool_format_01);
		return rthis().append(str[0], 1);
	}
	inline actual_t& operator%(char v)
	{
		return rthis().append(v, 1);
	}
	inline actual_t& operator%(int8_t v)
	{
		eco::c_str str(v);
		return rthis().append(str.value(), str.size());
	}
	inline actual_t& operator%(uint8_t v)
	{
		eco::c_str str(v);
		return rthis().append(str.value(), str.size());
	}
	inline actual_t& operator%(int16_t v)
	{
		eco::c_str str(v);
		return rthis().append(str.value(), str.size());
	}
	inline actual_t& operator%(uint16_t v)
	{
		eco::c_str str(v);
		return rthis().append(str.value(), str.size());
	}
	inline actual_t& operator%(int32_t v)
	{
		eco::c_str str(v);
		return rthis().append(str.value(), str.size());
	}
	inline actual_t& operator%(uint32_t v)
	{
		eco::c_str str(v);
		return rthis().append(str.value(), str.size());
	}
	inline actual_t& operator%(int64_t v)
	{
		eco::c_str str(v);
		return rthis().append(str.value(), str.size());
	}
	inline actual_t& operator%(uint64_t v)
	{
		eco::c_str str(v);
		return rthis().append(str.value(), str.size());
	}
	inline actual_t& operator%(float v)
	{
		eco::c_str str(v);
		return rthis().append(str.value(), str.size());
	}
	inline actual_t& operator%(double v)
	{
		eco::c_str str(v);
		return rthis().append(str.value(), str.size());
	}
	inline actual_t& operator%(const char* v)
	{
		return rthis().append(v, (uint32_t)strlen(v));
	}
	inline actual_t& operator%(const eco::string_view& v)
	{
		return rthis().append(v.c_str(), v.size());
	}
	inline actual_t& operator%(const eco::c_str& v)
	{
		return rthis().append(v.value(), v.size());
	}
	inline actual_t& operator%(const eco::string_c& v)
	{
		return rthis().append(v.c_str(), v.size());
	}
	inline actual_t& operator%(const std::string& v)
	{
		return rthis().append(v.c_str(), v.size());
	}

	inline format_t(const char* format = nullptr)
		: m_pos(0), m_format(format)
	{}

	inline actual_t& format(const char* v)
	{
		m_pos = 0;
		m_format = v;
		return rthis();
	}

protected:
	inline bool move_to_next_flag()
	{
		if (m_pos != static_cast<uint32_t>(-1))
		{
			const char* curr = m_format + m_pos;
			m_pos = eco::find_first(curr, '%');
			if (m_pos == static_cast<uint32_t>(-1))
			{
				rthis().append(curr);
				return false;
			}
			rthis().append(curr, m_pos++);
			m_pos += static_cast<uint32_t>(curr - m_format);
			return true;
		}
		return false;
	}

	uint32_t    m_pos;	
	const char* m_format;
};


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(eco)