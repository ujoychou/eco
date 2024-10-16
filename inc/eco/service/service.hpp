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
#include <eco/encoder/encoder.hpp>
#include <eco/encrypt/encrypt.hpp>
#include <eco/protocol/protocol.hpp>
#include <eco/service/remote.hpp>
#include <eco/service/method.hpp>


namespace eco {
////////////////////////////////////////////////////////////////////////////////
class service
{
public:
	template<typename protocol_t>
	inline service& protocol()
	{
		return *this;
	}

	template<typename encrypt_t>
	inline service& encrypt()
	{
		return *this;
	}

	template<typename encoder_t>
	inline service& encoder()
	{
		return *this;
	}

	inline eco::remote& remote()
	{
		return remote_;
	}

	inline eco::method method(int32_t method_id)
	{
		return eco::method();
	}

private:
	eco::remote remote_;
	std::unordered_map<int32_t, eco::method> method_map;
};



////////////////////////////////////////////////////////////////////////////////
} // namespace eco