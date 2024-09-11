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



namespace eco {
////////////////////////////////////////////////////////////////////////////////
class string : public eco::stream<eco::string>
{
protected:
	// init app config
	virtual void on_init() {}

	// init command
	virtual void on_cmd()  {}

	// load business data through the initialized object
	virtual void on_load() {}

	// app exit
	virtual void on_exit() {}

public:
	eco::service service(uint32_t domain, uint32_t service);
	eco::service service(uint32_t service);

	template<typename service_t>
	service_t service(uint32_t domain, uint32_t service);

	template<typename service_t>
	service_t service(uint32_t service);
};


#define ECO_APP(app_class)
////////////////////////////////////////////////////////////////////////////////
} // namespace eco