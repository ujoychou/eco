#include "PrecHeader.h"
#include "Inner.h"
////////////////////////////////////////////////////////////////////////////////
#include "Engine.ipp"
#include <iostream>


namespace eco{;
namespace cmd{;


//##############################################################################
//##############################################################################
void CdCommand::execute(IN eco::cmd::Context& context)
{
	Engine::Impl* impl = &engine().impl();

	// app group: default home group.
	if (context.size() == 0)
	{
		if (impl->m_curr_group != engine().root().group_set().at(1))
		{
			m_group = impl->m_curr_group;
			impl->m_curr_group = engine().root().group_set().at(1);
		}
	}
	// parent group
	else if (strcmp(context.at(0), "..") == 0)
	{
		Group parent = engine().current().parent();
		if (!parent.null())
		{
			m_group = impl->m_curr_group;
			impl->m_curr_group = parent;
		}
	}
	// child group.
	else
	{
		const char* param = context.at(0);
		Group chd = engine().current().find_group(param);
		if (chd.null())
		{
			EcoCout << "this is not a command: " << param;
			return;
		}
		m_group = impl->m_curr_group;
		impl->m_curr_group = chd;
	}
}
void CdCommand::revoke()
{
	if (!m_group.null())
	{
		std::swap(engine().impl().m_curr_group, m_group);
	}
}
void CdCommand::resume()
{
	if (!m_group.null())
	{
		std::swap(engine().impl().m_curr_group, m_group);
	}
}



//##############################################################################
//##############################################################################
void PwdCommand::execute(IN eco::cmd::Context& context)
{
	// app group: default home group.
	if (context.size() == 0)
	{
		std::string dir = pwd(engine().get_current());
		EcoCout << dir;
	}
}
void PwdCommand::revoke()
{}
void PwdCommand::resume()
{}
std::string PwdCommand::pwd(IN Group& group)
{
	std::vector<Group> vec;

	// group to root.
	Group temp = group;
	while (true)
	{
		vec.push_back(temp);
		if (!temp.has_parent())
		{
			break;
		}
		temp = temp.parent();
	}

	// format string.
	std::string msg;
	std::vector<Group>::reverse_iterator it;
	for (it = vec.rbegin(); it != vec.rend(); ++it)
	{
		msg += it->name();
		msg += "/";
	}
	return msg;
}



//##############################################################################
//##############################################################################
void HelpCommand::execute(IN eco::cmd::Context& context)
{
	std::vector<Class>& inner_cmds = engine().impl().m_inner_cmds;

	char fmt[512] = {0};
	if (!inner_cmds.empty())
	{
		auto it = inner_cmds.begin();
		for (; it != inner_cmds.end(); ++it)
		{
			snprintf(fmt, sizeof(fmt), "(c) %-10s %-20s: %s",
				it->alias(), it->name(), it->help_info());
			EcoCout << fmt;
		}
	}
	if (inner_cmds.empty())
	{
		EcoCout << "command engine has no inner command.";
	}
}
void HelpCommand::revoke()
{}
void HelpCommand::resume()
{}


//##############################################################################
//##############################################################################
void ListCommand::revoke()
{}
void ListCommand::resume()
{}


////////////////////////////////////////////////////////////////////////////////
}}