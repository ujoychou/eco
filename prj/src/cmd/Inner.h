#ifndef ECO_CMD_INNER_H
#define ECO_CMD_INNER_H
/*******************************************************************************
@ name
inner command.

@ function
1.cd command.

@ exception

@ note

--------------------------------------------------------------------------------
@ [history ver 1.0]
@ ujoy modifyed on 2016-05-06.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2017, ujoy, reserved all right.

*******************************************************************************/
#include <eco/cmd/Group.h>
#include <eco/cmd/Class.h>
#include <vector>




namespace eco{;
namespace cmd{;


////////////////////////////////////////////////////////////////////////////////
inline static bool equal(IN const Class& c, IN const std::string& name)
{
	return name == c.get_alias() || name == c.get_name();
}

////////////////////////////////////////////////////////////////////////////////
inline static bool equal_group(IN const Group& c, IN const std::string& name)
{
	return name == c.get_alias() || name == c.get_name();
}


////////////////////////////////////////////////////////////////////////////////
class CdCommand : public eco::cmd::Command
{
	ECO_COMMAND(CdCommand, "cd", "cd");
public:
	virtual void execute(
		IN const eco::cmd::Context& context);
	virtual void revoke();
	virtual void resume();

private:
	Group m_group;
};


////////////////////////////////////////////////////////////////////////////////
class PwdCommand : public eco::cmd::Command
{
	ECO_COMMAND(PwdCommand, "pwd", "pwd");
public:
	virtual void execute(
		IN const eco::cmd::Context& context);
	virtual void revoke();
	virtual void resume();
	std::string pwd(IN Group& group);
};


////////////////////////////////////////////////////////////////////////////////
class HelpCommand : public eco::cmd::Command
{
	ECO_COMMAND(HelpCommand, "help", "?");
public:
	virtual void execute(
		IN const eco::cmd::Context& context);
	virtual void revoke();
	virtual void resume();
};


////////////////////////////////////////////////////////////////////////////////
class ListCommand : public eco::cmd::Command
{
	ECO_COMMAND(ListCommand, "list", "ll");
public:
	virtual void execute(
		IN const eco::cmd::Context& context);
	virtual void revoke();
	virtual void resume();
};
class ShowCommand : public ListCommand
{
	ECO_COMMAND(ShowCommand, "list show", "ls");
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif