#include "PrecHeader.h"
#include <eco/cmd/Context.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/Project.h>
#include <vector>
#include <boost/algorithm/string.hpp>



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
ECO_VALUE_IMPL(Context);
ECO_PROPERTY_STR_IMPL(Context, command);
void Context::Impl::set_command_line(IN const char* cmd_line, IN Context& wrap)
{
	const char pre_val = ' ';
	std::string temp(cmd_line);

	// 解析命令名字：cmd
	size_t pos = temp.find(pre_val);
	if (pos == std::string::npos)
	{
		m_command = cmd_line;
		return ;
	}
	m_command = temp.substr(0, pos);

	// 解析参数: para1, para2, para3
	std::string val;
	size_t last_pos = pos + 1;
	pos = temp.find(pre_val, last_pos);
	while (pos != std::string::npos)
	{
		wrap.add().value() = temp.substr(last_pos, pos - last_pos).c_str();
		last_pos = pos + 1;
		pos = temp.find(pre_val, last_pos);
	}// end while

	int left_size = static_cast<int>(temp.size() - last_pos);
	if (left_size > 0)
	{
		wrap.add().value() = temp.substr(last_pos, left_size).c_str();
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