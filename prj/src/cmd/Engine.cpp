#include "PrecHeader.h"
#include "Engine.ipp"
////////////////////////////////////////////////////////////////////////////////
#include <eco/Project.h>
#include <iostream>
#include "Inner.h"


namespace eco{;
namespace cmd{;


//##############################################################################
//##############################################################################
std::string prompt(IN Group& group)
{
	// current group.
	std::string msg;
	msg += "[ ";
	msg += group.get_name();
	msg += " ] ";
	return msg;
}
void get_command_line(
	OUT std::string& cmd_line,
	IN  const std::string& user_prompt)
{
	std::cout << user_prompt;
	std::getline(std::cin, cmd_line);
}



//##############################################################################
//##############################################################################
ECO_TYPE_IMPL(Engine);
bool Engine::Impl::run_inner_command(IN const Context& context)
{
	std::vector<Class>::iterator it = std::find_if(
		m_inner_cmds.begin(), m_inner_cmds.end(),
		std::bind(&equal, std::placeholders::_1, context.get_command()));
	if (it == m_inner_cmds.end())
	{
		return false;
	}
	
	std::shared_ptr<Command> inner_cmd(it->create_command());
	inner_cmd->execute(context);
	return true;
}


////////////////////////////////////////////////////////////////////////////////
void Engine::Impl::work()
{	
	while (true)
	{
		// get user input.
		std::string cmd_line;
		get_command_line(cmd_line, prompt(m_curr_group));

		// get command context.
		Context context;
		context.set_command_line(cmd_line.c_str());

		// 1. empty command.
		if (context.is_empty())
		{
			continue;
		}

		// 2. exit command.
		if (context.is_exit())
		{
			break;
		}

		// 3. find whether it is a inner command.
		if (!run_inner_command(context))
		{
			// 4. run command in current command group.
			m_curr_group.run_command(context);
		}
		eco::cout(1);	// turn line.
	}
}


////////////////////////////////////////////////////////////////////////////////
void Engine::Impl::run()
{
	if (eco::empty(home().get_name()))
	{
		home().name("app");
	}
	m_thread.run(std::bind(&Engine::Impl::work, this), "cmd");
}


////////////////////////////////////////////////////////////////////////////////
Engine::Impl::Impl() : m_curr_group(eco::null) {}
void Engine::Impl::init(Engine&)
{
	// init engine: inner commands.
	m_inner_cmds.push_back(Class().bind<CdCommand>(
		"cd command group like cd directory on os."));
	m_inner_cmds.push_back(Class().bind<PwdCommand>(
		"pwd show current group."));
	m_inner_cmds.push_back(Class().bind<HelpCommand>(
		"help show inner command detail."));
	m_inner_cmds.push_back(Class().bind<ListCommand>(
		"list children group and command detail."));
	m_inner_cmds.push_back(Class().bind<ShowCommand>(
		"same with list command."));

	// init engine: root_group "root->sys/app;"
	m_root_group.name("root").alias("/");
	m_root_group.add_group().name("sys");
	m_curr_group = m_root_group.add_group().name("");
}


//##############################################################################
//##############################################################################
Group Engine::root()
{
	return m_impl->m_root_group;
}
const Group Engine::get_root() const
{
	return m_impl->m_root_group;
}
Group Engine::current()
{
	return m_impl->m_curr_group;
}
const Group Engine::get_current() const
{
	return m_impl->m_curr_group;
}
void Engine::run()
{
	m_impl->run();
}
void Engine::join()
{
	m_impl->m_thread.join();
}
Group Engine::home()
{
	return impl().home();
}


////////////////////////////////////////////////////////////////////////////////
Engine& get_engine()
{
	return eco::Singleton<Engine>::instance();
}

////////////////////////////////////////////////////////////////////////////////
}}