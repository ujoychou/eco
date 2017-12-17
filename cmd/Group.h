#ifndef ECO_CMD_GROUP_H
#define ECO_CMD_GROUP_H
/*******************************************************************************
@ name
command group.

@ function
1.command group can contain command group and command children.
2.command group and command just like directory and file.

@ exception

@ note

--------------------------------------------------------------------------------
@ [history ver 1.0]
@ ujoy modifyed on 2016-05-06.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2017, ujoy, reserved all right.

*******************************************************************************/
#include <eco/cmd/Class.h>
#include <eco/cmd/Command.h>






namespace eco{;
namespace cmd{;


class GroupSet;
////////////////////////////////////////////////////////////////////////////////
class ECO_API Group
{
	ECO_SHARED_API(Group);
public:
	/*@ run command with dedicated parameter.
	* @ params.context: command name and parameter.
	*/
	void run_command(
		IN const Context& context);

	/*@ is exit.*/
	bool is_exit_command(IN const char* cmd_name) const;

	/*@ group name.*/
	Group& name(IN const char* name);
	const char* get_name() const;

	/*@ group alias.*/
	Group& alias(IN const char* name);
	const char* get_alias() const;

	/*@ group name.*/
	Group& help_info(IN const char* name);
	const char* get_help_info() const;

	/*@ context mode: where command can be revoke and resume.*/
	Group& open_context(IN const bool = false);
	const bool context_openned() const;

	/*@ get parent group.*/
	Group get_parent() const;

	/*@ has parent group.*/
	bool has_parent() const;

	/*@ find group.*/
	Group find_group(
		IN const char* name) const;

	// get command set.
	CommandSet& command_set();
	const CommandSet& get_command_set() const;

	// get group set.
	GroupSet& group_set();
	const GroupSet& get_group_set() const;

public:
	// add command.
	void add_command(Class&);
	Class& add_command();

	// add group.
	Group& add_group();
	void add_group(Group&);
};


////////////////////////////////////////////////////////////////////////////////
class ECO_API GroupSet
{
	ECO_SHARED_API(GroupSet);
public:
	/*@ command group iterator.*/
	typedef eco::iterator<Group> iterator;
	typedef eco::iterator<const Group> const_iterator;
	iterator begin();
	const_iterator begin() const;
	iterator end();
	const_iterator end() const;

	/*@ add command group.*/
	void add(IN const Group&);
	Group& add_group();
	void push_back(IN const Group&);

	/*@ move & copy.*/
	void add_move(IN GroupSet&);
	void add_copy(IN const GroupSet&);

	/*@ remove command group.*/
	void erase(IN int);
	iterator erase(IN iterator& it);
	void pop_back();

	/*@ clear command group.*/
	void clear();

	/*@ get command group set size.*/
	size_t size() const;
	bool empty() const;
	void reserve(IN const size_t capacity);

	/*@ access command group by item index.*/
	Group& at(IN const int i);
	const Group& at(IN const int i) const;

private:
	Group& add();
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif