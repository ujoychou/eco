
#include "Pch.h"
#include <eco/cmd/Context.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/rx/RxImpl.h>


namespace eco{;
namespace cmd{;
////////////////////////////////////////////////////////////////////////////////
class Context::Impl
{
public:
	std::string m_command;

	void init(IN Context&) {}

	void set_command_line(
		IN const char* cmd_line,
		IN Context& wrap);
};


////////////////////////////////////////////////////////////////////////////////
ECO_VALUE_IMPL(Context, eco::Context);
ECO_PROPERTY_STR_IMPL(Context, command);
void Context::Impl::set_command_line(IN const char* cmd_line, IN Context& wrap)
{
	std::vector<std::string> set;
	eco::split(set, cmd_line, ' ');

	// 解析命令名字：cmd
	if (set.size() > 0)
	{
		m_command = set[0];
	}

	// 解析参数: para1, para2, para3
	for (size_t i = 1; i < set.size(); i++)
	{
		wrap.add().get_value() = set[i].c_str();
	}
}


////////////////////////////////////////////////////////////////////////////////
void Context::set_command_line(IN const char* cmd_line)
{
	impl().set_command_line(cmd_line, *this);
}
bool Context::is_exit() const
{
	return impl().m_command == "exit";
}
bool Context::is_empty() const
{
	return impl().m_command.empty();
}
bool Context::is_group_help() const
{
	return impl().m_command == "help"
		|| impl().m_command == "?"
		|| impl().m_command == "ll"
		|| impl().m_command == "ls";
}


////////////////////////////////////////////////////////////////////////////////
}}