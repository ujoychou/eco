#include "PrecHeader.h"
#include <eco/cmd/Group.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/Project.h>
#include <eco/log/Log.h>
#include <eco/Implement.h>
#include <eco/cmd/Engine.h>
#include <iostream>
#include "Inner.h"



namespace eco{;
namespace cmd{;


////////////////////////////////////////////////////////////////////////////////
class GroupSet::Impl
{
public:
	std::vector<Group> m_items;
	Group m_parent;

	Impl() : m_parent(eco::null) {}
	void init(IN GroupSet& wrap) {}
};


////////////////////////////////////////////////////////////////////////////////
class Group::Impl
{
public:
	std::string m_name;
	std::string m_alias;
	std::string m_help_info;

	// context managemnt.
	bool m_open_context;

	// command set.
	CommandSet m_command_set;

	// parent group & child group set.
	Group m_parent;
	GroupSet m_group_set;

	inline Impl() : m_parent(eco::null)
	{}

	void init(IN Group& wrap)
	{
		m_open_context = false;
		m_group_set.impl().m_parent = wrap;
	}
};


////////////////////////////////////////////////////////////////////////////////
ECO_SHARED_IMPL(GroupSet);
ECO_PROPERTY_SET_IMPL(GroupSet, Group);
ECO_SHARED_IMPL(Group);

////////////////////////////////////////////////////////////////////////////////
void ListCommand::execute(IN const eco::cmd::Context& context)
{
	Group::Impl& data = get_engine().current().impl();

	// group info.
	char fmt[512] = {0};
	if (!data.m_group_set.empty())
	{
		auto it = data.m_group_set.begin();
		for (; it!=data.m_group_set.end(); ++it)
		{
			snprintf(fmt, sizeof(fmt), "{g} %-8s %-15s: %s",
				it->get_alias(), it->get_name(), it->get_help_info());
			EcoCout << fmt;
		}
	}

	// command info.
	if (!data.m_command_set.empty())
	{
		auto it = data.m_command_set.begin();
		for (; it != data.m_command_set.end(); ++it)
		{
			snprintf(fmt, sizeof(fmt), "(c) %-8s %-15s: %s",
				it->get_alias(), it->get_name(), it->get_help_info());
			EcoCout << fmt;
		}
	}
	if (data.m_command_set.empty() && data.m_group_set.empty())
	{
		EcoCout << "this group has no command.";
	}
}



////////////////////////////////////////////////////////////////////////////////
Group& GroupSet::add_group()
{
	Group it;
	it.impl().m_parent = impl().m_parent;
	impl().m_items.push_back(it);
	return impl().m_items.back();
}


////////////////////////////////////////////////////////////////////////////////
void run_command(IN const Context& context, OUT Group::Impl& data)
{
	std::vector<int>::iterator its;
	// find command class.
	auto it = std::find_if(data.m_command_set.begin(), data.m_command_set.end(),
		std::bind(&equal, std::placeholders::_1, context.get_command()));
	if (it == data.m_command_set.end())
	{
		EcoCout << "can't find command: " << context.get_command();
		return ;
	}

	// run command.
	try 
	{
		std::shared_ptr<Command> command(it->create_command());
		command->execute(context);
	}
	catch (eco::Error& e)
	{
		EcoError << "execute command error: " << e;
	}
	catch (std::exception& e)
	{
		EcoError << "execute command error: " << e.what();
	}
}


////////////////////////////////////////////////////////////////////////////////
void Group::run_command(IN const Context& param)
{
	eco::cmd::run_command(param, impl());
}
bool Group::is_exit_command(IN const char* cmd_name) const
{
	return strcmp(cmd_name, "exit") == 0;
}
Group& Group::name(IN const char* name)
{
	impl().m_name = name;
	return *this;
}
const char* Group::get_name() const
{
	return impl().m_name.c_str();
}
Group& Group::alias(IN const char* alias)
{
	impl().m_alias = alias;
	return *this;
}
const char* Group::get_alias() const
{
	return impl().m_alias.c_str();
}
Group& Group::help_info(IN const char* name)
{
	impl().m_help_info = name;
	return *this;
}
const char* Group::get_help_info() const
{
	return impl().m_help_info.c_str();
}
Group& Group::open_context(IN bool is_open)
{
	impl().m_open_context = is_open;
	return *this;
}
const bool Group::context_openned() const
{
	return impl().m_open_context;
}
Group Group::get_parent() const
{
	return impl().m_parent;
}
bool Group::has_parent() const
{
	return !impl().m_parent.null();
}
Group Group::find_group(IN const char* name) const
{
	auto it = std::find_if(
		impl().m_group_set.begin(), impl().m_group_set.end(),
		std::bind(&equal_group, std::placeholders::_1, name));
	return (it != impl().m_group_set.end()) ? *it : Group(eco::null);
}
CommandSet& Group::command_set()
{
	return impl().m_command_set;
}
const CommandSet& Group::get_command_set() const
{
	return impl().m_command_set;
}
GroupSet& Group::group_set()
{
	return impl().m_group_set;
}
const GroupSet& Group::get_group_set() const
{
	return impl().m_group_set;
}
Class& Group::add_command()
{
	return command_set().add();
}
void Group::add_command(Class& c)
{
	return command_set().add(c);
}
Group& Group::add_group()
{
	return group_set().add_group();
}
void Group::add_group(Group& c)
{
	return group_set().add(c);
}
////////////////////////////////////////////////////////////////////////////////
}}