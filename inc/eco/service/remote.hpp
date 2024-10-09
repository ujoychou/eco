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
#include <stdint.h>


namespace eco {
////////////////////////////////////////////////////////////////////////////////
class remote
{
public:
	template<typename encoder_t>
	inline remote& encoder()
	{
		return *this;
	}

	template<typename encrypt_t>
	inline remote& encrypt()
	{
		return *this;
	}

	template<typename protocol_t>
	inline remote& protocol()
	{
		return *this;
	}
};


////////////////////////////////////////////////////////////////////////////////
} // namespace eco