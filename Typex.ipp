#ifndef ECO_TYPEX_IPP
#define ECO_TYPEX_IPP
/*******************************************************************************
@ name

@ function


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2015-01-15.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2015 - 2017, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Project.h>
#include <eco/Typex.h>


namespace eco{;


////////////////////////////////////////////////////////////////////////////////
class StringAny::Impl
{
public:
	void init(IN StringAny&){}
	std::string m_value;

public:
	Impl() {}
	Impl(IN const char* v) : m_value(v) {}
};


////////////////////////////////////////////////////////////////////////////////
class StdString::Impl
{
	ECO_IMPL_INIT(StdString);
public:
	std::string m_value;
};
class StdStringSet::Impl
{
	ECO_IMPL_INIT(StdStringSet);
public:
	std::vector<StdString> m_items;
};


////////////////////////////////////////////////////////////////////////////////
class Parameter::Impl
{
public:
	std::string m_key;
	StringAny m_value;

	void init(IN Parameter&) {}
};


////////////////////////////////////////////////////////////////////////////////
class Context::Impl
{
public:
	std::vector<Parameter> m_items;

	void init(IN Context&) {}
};


////////////////////////////////////////////////////////////////////////////////
class ContextNode::Impl
{
public:
	void init(ContextNode&) {}
	std::string  m_name;
	std::string  m_value;
	eco::Context m_property_set;
	eco::ContextNodeSet m_children;

public:
	Impl() : m_children(eco::null) {}
};
class ContextNodeSet::Impl
{
public:
	void init(ContextNodeSet&) {}
	std::vector<ContextNode> m_items;
};

////////////////////////////////////////////////////////////////////////////////
}// ns
#endif