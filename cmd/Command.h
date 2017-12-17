#ifndef ECO_CMD_COMMAND_H
#define ECO_CMD_COMMAND_H
/*******************************************************************************
@ name
command that input by user on service console.

@ function
1.command info.
2.command runner.

@ exception

@ note

--------------------------------------------------------------------------------
@ [history ver 1.0]
@ ujoy modifyed on 2016-05-06.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2017, ujoy, reserved all right.

*******************************************************************************/
#include <eco/cmd/Context.h>
#include <eco/HeapOperators.h>


#define ECO_COMMAND(cmd_class, cmd_name, cmd_alias)\
public:\
inline static Command* create()\
{\
	return new cmd_class();\
}\
inline static const char* get_name()\
{\
	return cmd_name;\
}\
inline static const char* get_alias()\
{\
	return cmd_alias;\
}\
private:

namespace eco{;
namespace cmd{;


enum ExecuteMode
{
	execute_mode,
	revoke_mode,
	resume_mode,
};


// create command.
class Command;
typedef Command* (*CreateCommandFunc)(void);

// command execute function.
typedef void (*ExecuteFunc)(
	IN const eco::cmd::Context&,
	IN eco::cmd::ExecuteMode);

// command execute functor.
typedef std::function<void(
	IN const eco::cmd::Context&,
	IN eco::cmd::ExecuteMode)> CommandExecute;

////////////////////////////////////////////////////////////////////////////////
class ECO_API Command : public eco::HeapOperators
{
	ECO_NONCOPYABLE(Command);
public:
	inline Command() {}

	/*@ run this command with parameters.
	* @ params.context: command context that contain parameters.
	*/
	virtual void execute(
		IN const eco::cmd::Context& context) = 0;

	// cancel execute command.
	virtual void revoke() {};

	// redo execute command.
	virtual void resume() {};
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif