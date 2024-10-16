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
#include <eco/string/stream.hpp>



eco_namespace(eco)
////////////////////////////////////////////////////////////////////////////////
class string : public eco::stream<eco::string>
{
public:
	inline void append(const char c, uint32_t size)
	{
	}

	inline void append(const char* str, uint32_t size)
	{
	}
};


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(eco)