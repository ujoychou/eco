#ifndef ECO_CMD_ENGINE_H
#define ECO_CMD_ENGINE_H
/*******************************************************************************
@ name
command engine.

@ function
1.has a thread to run the command.
2.manage command and command group.
3.precommand is the sufficient condition of current command, so when call a 
command, ensure that it's precommand must be finished. or it will call 
precommands by order.

@ exception

@ note

--------------------------------------------------------------------------------
@ [history ver 1.0]
@ ujoy modifyed on 2016-05-06.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2018, ujoy, reserved all right.

*******************************************************************************/
#include <eco/cmd/Group.h>
#include <eco/Object.h>


namespace eco{;
namespace cmd{;


////////////////////////////////////////////////////////////////////////////////
class ECO_API Engine
{
	ECO_IMPL_API();
	ECO_SINGLETON_UNINIT(Engine);
public:
	/*@ run command engine.*/
	void run();

	void join();

	/*@ get command root group.*/
	Group root();
	const Group get_root() const;

	/*@ get current command group.*/
	Group current();
	const Group get_current() const;

	/*@ get home.*/
	Group home();
};


ECO_API Engine& get_engine();
////////////////////////////////////////////////////////////////////////////////
}}
#endif