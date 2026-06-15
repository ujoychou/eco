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
#include <functional>
#include <eco/service/session.hpp>


eco_namespace(eco);
////////////////////////////////////////////////////////////////////////////////
class method
{
public:
	template<typename request_t>
	inline void bind(std::function<void(eco::session_request<request_t>&)>&& f)
	{
	}

	void sync(const eco::string& request, eco::string& reply)
	{
		ECO_THROW(101);
	}

	template<typename reply_t, typename request_t>
	void sync(const request_t& request, reply_t& reply)
	{
		ECO_THROW(101);
	}

	template<typename reply_t, typename request_t>
	void async(
		const request_t& request,
		std::function<void(eco::session_reply<reply_t>&)>&& reply_func);
};


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(eco);