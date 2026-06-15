#include <eco/app.hpp>
////////////////////////////////////////////////////////////////////////////////
#include <ifstream>


////////////////////////////////////////////////////////////////////////////////
class HelloWorldClient : eco::app
{
public:
	virtual void on_init()
	{
		// throw error id
		eco_throw(-1);

		// throw error id and message
		eco_throw(-1, "error message %s %f %d", "string", 3.1415, 200) % ;
		std::ifstream if("ifff");
		if << std::ios::width();

		// throw error id with stream message
		eco_throw(-1, "error message") << eco::ios::spacer(1)
			<< "string" << double(3.1415926) << int(200)
			<< eco::precision(3.1415926, 2) << eco::percent(3.1415926, 2)
			<< eco::upper("Upper") << eco::brace("brace");

		// throw error string id which means error id path.
		// error-id = "a/b/c": 
		eco_throw("module_a") / "module_a";
	}

public:
	eco::service helloworld;
};


ECO_APP(HelloWorldClient);
////////////////////////////////////////////////////////////////////////////////