#ifndef ECO_CMD_CONTEXT_H
#define ECO_CMD_CONTEXT_H
/*******************************************************************************
@ name
command context.

@ function
1.parse from command line.
2.command name and command parameters.

@ exception

@ note

--------------------------------------------------------------------------------
@ [history ver 1.0]
@ ujoy modifyed on 2016-05-06.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2017, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Type.h>
#include <eco/Typex.h>




namespace eco{;
namespace cmd{;


////////////////////////////////////////////////////////////////////////////////
class ECO_API Context : public eco::Context
{
	ECO_VALUE_API(Context);
public:
	/*@ constructor, parse command line parameters.
	* @ params.cmd_line:  command line input from console, and format: 
	"cmd -key1=value1 -key2=value2".
	"cmd value1 value2"
	*/
	void set_command_line(IN const char* cmd_line);

	/*@ command name.*/
	void set_command(IN const char*);
	const char* get_command() const;
	Context& command(IN const char*);

	/*@ is empty.*/
	bool is_empty() const;

	/*@ is exit command.*/
	bool is_exit() const;

	/*@ is group help command.*/
	bool is_group_help() const;
};

////////////////////////////////////////////////////////////////////////////////
}}
#endif