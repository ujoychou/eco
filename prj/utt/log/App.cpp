#include "PrecHeader.h"
#include "App.h"
////////////////////////////////////////////////////////////////////////////////
#include <eco/Project.h>
#include <eco/log/Log.h>
#include <eco/thread/Thread.h>
#include "Test.h"

namespace eco{;
namespace log{;
namespace test{;


////////////////////////////////////////////////////////////////////////////////
void input_log(IN const eco::cmd::Context& context, IN eco::cmd::ExecuteMode)
{
	if (context.size() > 0)
	{
		EcoInfo << context.at(0).get_value().c_str();
	}
}


////////////////////////////////////////////////////////////////////////////////
App::App()
{
	eco::App::set_log_file_on_changed(&OnChangedLogFile);
}


////////////////////////////////////////////////////////////////////////////////
void App::on_cmd()
{
	eco::App::home().add_command().bind<MtCommand>(
		"multi thread logging test. [mt 100]");
	eco::App::home().add_command().bind<FuncCommand>(
		"logging function test. [fc Info D1]");
	eco::App::home().add_command().bind(
		"input", "i", "input a string to logging.",
		std::bind(&input_log, std::placeholders::_1, std::placeholders::_2));
}


////////////////////////////////////////////////////////////////////////////////
void App::OnChangedLogFile(IN const char* file_path)
{
	EcoCout << "log file on changed: " << file_path;
}
////////////////////////////////////////////////////////////////////////////////
}}}