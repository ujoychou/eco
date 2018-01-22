#ifndef ECO_CMD_ENGINE_IPP
#define ECO_CMD_ENGINE_IPP
/*******************************************************************************
@ name
cmd engine ipp.

@ function

@ exception

@ note

--------------------------------------------------------------------------------
@ [history ver 1.0]
@ ujoy modifyed on 2016-05-06.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2017, ujoy, reserved all right.

*******************************************************************************/
#include <eco/cmd/Engine.h>
#include <eco/cmd/Group.h>
#include <eco/cmd/Class.h>
#include <eco/thread/Thread.h>
#include <vector>




namespace eco{;
namespace cmd{;


////////////////////////////////////////////////////////////////////////////////
class Engine::Impl
{
public:
	Group m_root_group;
	Group m_curr_group;
	eco::Thread m_thread;
	std::vector<Class> m_inner_cmds;

public:
	Impl();
	void init(Engine&);
	void work();
	void run();
	bool run_inner_command(IN const Context& context);

	inline Group home()
	{
		// app group index is 1; and sys group index is 0;
		return m_root_group.group_set().at(1);
	}
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif