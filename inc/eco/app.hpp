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
#include <eco/service/service.hpp>



namespace eco {
////////////////////////////////////////////////////////////////////////////////
class app
{
	ECO_SINGLE(app);
protected:
	app();

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


class config
{
};

class config_xml : public config
{
};

class config_json : public config
{
};

void main()
{
	config/locale using static memory cache defined in the dll;
	dll: config/locale/logging/command
	plugin-erx: 
	config* conf;

	"log message with a async thread, which will not be writed in file realtime when log_api return.";
	const char* name = "logging/async";
	conf->define(name, true, eco::locale().dictionary().value(name), );
	conf->define("locale/default@", );
}

C__语言

eco_throw(...) eco::error(#ARGS).trace(__func__, __line__)
eco_trace(...) eco::error(#ARGS).trace(__func__, __line__)


// eco::error重新赋值时，会重置错误信息，并清空错误调用堆栈。

bool f1()
{
	return eco_trace("trade/accout/E001") << account;
}

bool f2()
{
	if (!f1())
	{
		return eco_trace();
	}
}


void main()
{
	if (!f2())
	{
		eco_error() << eco_trace();
	}
}

void main()
{
	try
	{
		f3();
	}
	catch (const eco::error& e)
	{
		eco_error() << e;
	}
}

void on_request()
{
	try
	{
		f3();
	}
	catch (const eco::error& e)
	{
		reject(e);
	}
}

#define ECO_APP(app_class)
////////////////////////////////////////////////////////////////////////////////
} // namespace eco