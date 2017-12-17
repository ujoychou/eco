#include "PrecHeader.h"
#include <eco/cmd/Class.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/Project.h>
#include <vector>



namespace eco{;
namespace cmd{;


////////////////////////////////////////////////////////////////////////////////
class FuncCommand : public Command
{
public:
	class Impl;

	explicit FuncCommand(IN CommandExecute& func) : m_execute(func)
	{}
	~FuncCommand();

	// execute func.
	virtual void execute(
		IN const eco::cmd::Context& context) override
	{
		m_execute(context, execute_mode);
	}

	// cancel execute.
	virtual void revoke() override
	{
		m_execute(m_context, revoke_mode);
	}

	// resume execute.
	virtual void resume() override
	{
		m_execute(m_context, resume_mode);
	}

private:
	Context m_context;
	CommandExecute m_execute;
};


////////////////////////////////////////////////////////////////////////////////
class Class::Impl
{
public:
public:
	std::string m_name;
	std::string m_alias;
	std::string m_help_info;
	CreateCommandFunc  m_create;
	CommandExecute m_execute;

	void init(IN Class&)
	{
		m_create = nullptr;
		m_execute = nullptr;
	}
};




////////////////////////////////////////////////////////////////////////////////
class CommandSet::Impl
{
public:
public:
	std::vector<Class> m_items;

	void init(CommandSet&) {}
};


////////////////////////////////////////////////////////////////////////////////
ECO_SHARED_IMPL(Class);
ECO_PROPERTY_STR_IMPL(Class, name);
ECO_PROPERTY_STR_IMPL(Class, alias);
ECO_PROPERTY_STR_IMPL(Class, help_info);
ECO_SHARED_IMPL(CommandSet);
ECO_PROPERTY_SET_IMPL(CommandSet, Class);


////////////////////////////////////////////////////////////////////////////////
Command* Class::create_command()
{
	if (impl().m_create != nullptr)
	{
		return impl().m_create();
	}
	if (impl().m_execute != nullptr)
	{
		return new FuncCommand(impl().m_execute);
	}
	return nullptr;
}
Class& Class::bind(
	IN const char* name,
	IN const char* alias,
	IN const char* help_info,
	IN CreateCommandFunc func)
{
	if (name != nullptr)
		set_name(name);
	if (alias != nullptr)
		set_alias(alias);
	if (help_info != nullptr)
		set_help_info(help_info);

	impl().m_create = func;
	impl().m_execute = nullptr;
	return *this;
}
Class& Class::bind(
	IN const char* name,
	IN const char* alias,
	IN const char* help_info,
	IN ExecuteFunc func)
{
	if (name != nullptr)
		set_name(name);
	if (alias != nullptr)
		set_alias(alias);
	if (help_info != nullptr)
		set_help_info(help_info);

	impl().m_create = nullptr;
	impl().m_execute = func;
	return *this;
}
Class& Class::bind(
	IN const char* name,
	IN const char* alias,
	IN const char* help_info,
	IN CommandExecute func)
{
	if (name != nullptr)
		set_name(name);
	if (alias != nullptr)
		set_alias(alias);
	if (help_info != nullptr)
		set_help_info(help_info);

	impl().m_create = nullptr;
	impl().m_execute = func;
	return *this;
}


////////////////////////////////////////////////////////////////////////////////
}}