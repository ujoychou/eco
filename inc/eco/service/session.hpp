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

#define eco_namespace(ns) namespace ns {
#define eco_namespace_end(ns) }

namespace eco {
////////////////////////////////////////////////////////////////////////////////
template<typename message_t>
class session
{
public:
};


template<typename request_t>
class session_request
{
public:
	inline message_t& request()
	{
		return req;
	}

	template<typename reply_t>
	inline void reply(const reply_t& msg)
	{
	}

	inline void reject()
	{
	}

private:
	request_t req;
};


template<typename message_t>
class session_reply
{
public:
	bool has_error();

	inline message_t& reply()
	{
		return rsp;
	}

private:
	message_t rsp;
};


////////////////////////////////////////////////////////////////////////////////
} // namespace eco