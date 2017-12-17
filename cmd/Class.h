#ifndef ECO_CMD_CLASS_H
#define ECO_CMD_CLASS_H
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
#include <eco/cmd/Command.h>




namespace eco{;
namespace cmd{;



////////////////////////////////////////////////////////////////////////////////
class ECO_API Class
{
	ECO_SHARED_API(Class);
public:
	/*@ create and return a command instance.*/
	Command* create_command();

public:
	// add a concrete command inherit from command.
	template<typename T>
	inline Class& bind(IN const char* help_info)
	{
		return bind(T::get_name(), T::get_alias(), help_info, T::create);
	}

	// add a concrete command with a execute func.
	Class& bind(
		IN const char* name,
		IN const char* alias,
		IN const char* help_info,
		IN ExecuteFunc func);

	// add a concrete command with a execute func.
	Class& bind(
		IN const char* name,
		IN const char* alias,
		IN const char* help_info,
		IN CommandExecute func);

	/*@ command name which can be executed from command line.*/
	void set_name(IN const char*);
	const char* get_name() const;
	Class& name(IN const char*);

	/*@ command alias which can be executed from command line.*/
	void set_alias(IN const char*);
	const char* get_alias() const;
	Class& alias(IN const char*);

	/*@ help info.*/
	void set_help_info(IN const char*);
	const char* get_help_info() const;
	Class& help_info(IN const char*);

private:
	Class& bind(
		IN const char* name,
		IN const char* alias,
		IN const char* help_info,
		IN CreateCommandFunc func);
};


////////////////////////////////////////////////////////////////////////////////
class ECO_API CommandSet
{
	ECO_SHARED_API(CommandSet);
public:
	/*@ command iterator.*/
	typedef eco::iterator<Class> iterator;
	typedef eco::iterator<const Class> const_iterator;
	iterator begin();
	const_iterator begin() const;
	iterator end();
	const_iterator end() const;

	/*@ move & copy.*/
	void add_move(IN CommandSet&);
	void add_copy(IN const CommandSet&);

	/*@ add command.*/
	Class& add();
	void add(IN const Class&);
	void push_back(IN const Class&);

	/*@ remove command.*/
	void erase(IN int);
	iterator erase(IN iterator& it);
	void pop_back();

	/*@ clear command.*/
	void clear();

	/*@ get command set size.*/
	size_t size() const;
	bool empty() const;
	void reserve(IN const size_t capacity);

	/*@ access command by item index.*/
	Class& at(IN const int i);
	const Class& at(IN const int i) const;
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif