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


ECO_NS_BEGIN(eco);
namespace cmd{;


////////////////////////////////////////////////////////////////////////////////
class ECO_API Engine
{
	ECO_SINGLETON_API(Engine);
public:
	/*@ run command engine.*/
	void work();
	void start();

	void join();

	/*@ get command root group.*/
	Group get_root();
	const Group root() const;

	/*@ get current command group.*/
	Group get_current();
	const Group current() const;

	/*@ get home.*/
	Group home();
};


ECO_API Engine& engine();
////////////////////////////////////////////////////////////////////////////////
}}
#endif