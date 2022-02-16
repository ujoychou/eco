#ifndef ECO_CONFIG_H
#define ECO_CONFIG_H
/*******************************************************************************
@ name
key value config data.

@ function
1.support read from config xml.
2.supoort read from config ini.

@ exception

@ note

--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2016-05-09.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2018, ujoy, reserved all right.

*******************************************************************************/
#include <eco/String.h>
#include <eco/rx/RxExport.h>
#include <vector>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
class Parameter
{
public:
	inline Parameter() {}

	/*@ name.*/
	inline void set_name(IN const char*);
	inline const char* name() const;
	inline Parameter& name(IN const char*);

	/*@ value.*/
	inline void set_value(IN const StringAny& val);
	inline StringAny& get_value();
	inline void set_value(IN const char* val);
	inline const StringAny& value() const;
	inline Parameter& value(IN const StringAny& val);
	inline Parameter& value(IN const char* val);

public:
	inline operator const char*() const;
	inline operator char() const;
	inline operator unsigned char() const;
	inline operator int16_t() const;
	inline operator uint16_t() const;
	inline operator int32_t() const;
	inline operator uint32_t() const;
	inline operator int64_t() const;
	inline operator uint64_t() const;
	inline operator float() const;
	inline operator double() const;
	inline operator bool() const;

private:
	std::string m_name;
	eco::StringAny m_value;
	inline Parameter& impl() { return *this; }
	inline const Parameter& impl() const { return *this; }
};


////////////////////////////////////////////////////////////////////////////////
class ECO_API Context
{
	ECO_VALUE_API(Context);
public:
	/*@ parameter iterator.*/
	typedef eco::iterator<Parameter> iterator;
	typedef eco::iterator<const Parameter> const_iterator;
	iterator begin();
	const_iterator begin() const;
	iterator end();
	const_iterator end() const;

	/*@ add parameter.*/
	Parameter& add();
	void add(IN const Parameter&);
	void push_back(IN const Parameter&);

	/*@ move & copy.*/
	void add_move(IN Context&);
	void add_copy(IN const Context&);

	/*@ remove parameter.*/
	void erase(IN int);
	iterator erase(IN iterator& it);
	void pop_back();

	/*@ clear parameter.*/
	void clear();

	/*@ get context parameter set size.*/
	size_t size() const;
	bool empty() const;
	void reserve(IN size_t capacity);

	/*@ access parameter by item index.*/
	Parameter& at(IN int i);
	const Parameter& at(IN int i) const;

public:
	// whether has a key.
	bool has(IN const char* key) const;

	// get the key value.
	const StringAny& at(IN const char* key) const;

	// get the key value.
	const StringAny* find(IN const char* key) const;

	// get the key value.
	const StringAny get(IN const char* key) const;
};


////////////////////////////////////////////////////////////////////////////////
class ContextNodeSet;
class ECO_API ContextNode
{
	ECO_SHARED_API(ContextNode);
public:
	void set_name(IN const char*);
	const char* name() const;
	ContextNode& name(IN const char*);

	void set_value(IN const char*);
	const char* value() const;
	ContextNode& value(IN const char*);

	void set_property_set(IN const eco::Context&);
	eco::Context& get_property_set();
	const eco::Context& property_set() const;
	ContextNode& property_set(IN const eco::Context&);

	// get children
	bool has_children() const;
	void set_children(IN const eco::ContextNodeSet&);
	eco::ContextNodeSet& get_children();
	const eco::ContextNodeSet& children() const;
	eco::ContextNodeSet get_children(const char* child_key) const;

	// get child node.
	eco::ContextNode get_child(const char* child_key) const;

	// get children
	bool get_property_set(eco::Context& context, const char* key) const;

	// get key value.
	const StringAny& at(IN const char* key) const;
	const StringAny* find(IN const char* key) const;
	const StringAny  get(IN const char* key) const;

	// merge context node.
	void merge(eco::ContextNode& node);
};


////////////////////////////////////////////////////////////////////////////////
class ECO_API ContextNodeSet
{
	ECO_SHARED_API(ContextNodeSet);
public:
	/*@ parameter iterator.*/
	typedef eco::iterator<ContextNode> iterator;
	typedef eco::iterator<const ContextNode> const_iterator;
	iterator begin();
	const_iterator begin() const;
	iterator end();
	const_iterator end() const;

	/*@ move & copy.*/
	void add_move(IN ContextNodeSet&);
	void add_copy(IN const ContextNodeSet&);

	/*@ add parameter.*/
	ContextNode& add();
	void add(IN const ContextNode&);
	void push_back(IN const ContextNode&);

	/*@ remove parameter.*/
	void erase(IN int);
	iterator erase(IN iterator& it);
	void pop_back();

	/*@ clear parameter.*/
	void clear();

	/*@ get context parameter set size.*/
	size_t size() const;
	bool empty() const;
	void reserve(IN size_t capacity);

	/*@ access parameter by item index.*/
	ContextNode& at(IN int i);
	const ContextNode& at(IN int i) const;

	/*@ access parameter by item name.*/
	ContextNode* find(IN const char* name);
	const ContextNode* find(IN const char* name) const;
};


////////////////////////////////////////////////////////////////////////////////
class Context::Impl
{
	ECO_IMPL_INIT(Context);
public:
	std::vector<Parameter> m_items;
};
class ContextNode::Impl
{
	ECO_IMPL_INIT(ContextNode);
public:
	std::string  m_name;
	std::string  m_value;
	eco::Context m_property_set;
	eco::ContextNodeSet m_children;

public:
	inline Impl() : m_children(eco::null) {}
};
class ContextNodeSet::Impl
{
	ECO_IMPL_INIT(ContextNodeSet);
public:
	std::vector<ContextNode> m_items;
};


////////////////////////////////////////////////////////////////////////////////
class ECO_API Config
{
	ECO_SHARED_API(Config);
public:
	/*@ read config data from file.*/
	void init(IN const char* file);

	/*@ read config data from xml text.*/
	void init(IN const char* text, IN uint32_t size);

	/*@ get import file by index.*/
	uint32_t import_file_size() const;
	const char* import_file(IN uint32_t index) const;

	/*@ find key return true, else false.*/
	bool find(
		OUT eco::StringAny& v,
		IN const char* key) const;

	/*@ get value.*/
	const eco::StringAny at(
		IN const char* key) const;

	/*@ get value.*/
	const eco::StringAny get(
		IN const char* key) const;

	/*@ add key and value.*/
	void add(
		IN const char* key,
		IN const char* value);

	/*@ get children node set.*/
	eco::ContextNodeSet get_children(
		IN const char* parent_key = nullptr) const;
	eco::ContextNodeSet find_children(
		IN const char* parent_key = nullptr) const;

	/*@ get child node.*/
	eco::ContextNode get_node(
		IN const char* node_key = nullptr) const;
	eco::ContextNode find_node(
		IN const char* node_key = nullptr) const;

	/*@ get child node.*/
	eco::ContextNode find_node(
		IN const uint32_t index,
		IN const char* parent_key = nullptr) const;

	/*@ get node property.*/
	void get_property_set(
		OUT eco::Context& context_set,
		IN  const char* node_key = nullptr) const;

	/*@ has key return true, else false.*/
	inline bool has(IN const char* key) const
	{
		eco::StringAny v;
		return find(v, key);
	}
};


////////////////////////////////////////////////////////////////////////////////
void Parameter::set_name(IN const char* val)
{
	impl().m_name = val;
}
Parameter& Parameter::name(IN const char* val)
{
	impl().m_name = val;
	return *this;
}
const char* Parameter::name() const
{
	return impl().m_name.c_str();
}


////////////////////////////////////////////////////////////////////////////////
void Parameter::set_value(IN const StringAny& val)
{
	impl().m_value = val;
}
Parameter& Parameter::value(IN const StringAny& val)
{
	impl().m_value = val;
	return *this;
}
StringAny& Parameter::get_value()
{
	return impl().m_value;
}
const StringAny& Parameter::value() const
{
	return impl().m_value;
}
void Parameter::set_value(IN const char* val)
{
	m_value = val;
}
Parameter& Parameter::value(IN const char* val)
{
	m_value = val;
	return *this;
}
Parameter::operator const char*() const
{
	return m_value.c_str();
}
Parameter::operator char() const
{
	return (char)(m_value);
}
Parameter::operator unsigned char() const
{
	return (unsigned char)(m_value);
}
Parameter::operator int16_t() const
{
	return (int16_t)(m_value);
}
Parameter::operator uint16_t() const
{
	return (uint16_t)(m_value);
}
Parameter::operator int32_t() const
{
	return (int32_t)(m_value);
}
Parameter::operator uint32_t() const
{
	return (uint32_t)(m_value);
}
Parameter::operator int64_t() const
{
	return (int64_t)(m_value);
}
Parameter::operator uint64_t() const
{
	return (uint64_t)(m_value);
}
Parameter::operator float() const
{
	return (float)(m_value);
}
Parameter::operator double() const
{
	return (double)(m_value);
}
Parameter::operator bool() const
{
	return m_value;
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif