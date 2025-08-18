#pragma once
/*******************************************************************************
@ name
runtime type.

@ function
1. runtime type. exp: mfc runtime create.


@ exception

@ note

--------------------------------------------------------------------------------
@ [history ver 1.0]
@ ujoy modifyed on 2021-11-20


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2025, ujoy, reserved all right.

*******************************************************************************/
#include <eco/rtti/api.hpp>
#include <eco/string/string_c.hpp>


eco_namespace(eco);
eco_namespace(rtti);
class type;
////////////////////////////////////////////////////////////////////////////////
class object
{
	eco_object(object);
public:
	inline object() {}
	virtual ~object(void) {}

	// get class info of this object.
	virtual const eco::rtti::type* get_type() const = 0;

	// get the class info of this object.
	inline static const eco::rtti::type* type()
	{
		return nullptr;
	}

	inline static const char* type_name()
	{
		return eco_macro_str_(object);
	}

	template<typename object_t>
	inline static object::ptr cast(eco::rtti::object::ptr& obj)
	{
		return std::dynamic_pointer_cast<object_t>(obj);
	}

	// check is the kind of class.
	inline bool kind_of(const object& obj) const;
	template<typename object_t> inline bool kind_of() const;

	// check is the same class.
	inline bool same_of(const char* name) const;
	template<typename object_t> inline bool same_of() const;
};


////////////////////////////////////////////////////////////////////////////////
class eco_api type_registry
{
public:
	static void set_type(const char* name, const eco::rtti::type* type);
	static const eco::rtti::type* get_type(const char* name);
	static eco::rtti::object::ptr create(const char* name);
};

// type init when compile time.
typedef eco::rtti::object::ptr (*create_func_t)(void);
template<typename object_t, typename parent_t, create_func_t f>
struct type_init { static eco::rtti::type type; };
template<typename object_t, typename parent_t, create_func_t f>
eco::rtti::type type_init<object_t, parent_t, f>::type(
	object_t::type_name(), parent_t::type(), f);


////////////////////////////////////////////////////////////////////////////////
class type
{
	eco_noncopyable(type);
public:
	// init.
	inline type(const char* name, const type* parent, create_func_t create)
		: m_name(name), m_parent(parent), m_create(create)
	{
		rtti::type_registry::set_type(name, this);
	}

	// get type name of this type.
	inline const char* name() const { return m_name; }

	// get parent of this type.
	inline const eco::rtti::type* parent() const { return m_parent; }

	// is kind of runtime class.
	inline bool kind_of(const eco::rtti::type* type) const
	{
		const eco::rtti::type* sup = this;
		for (; sup != nullptr && type != sup; sup = sup->m_parent) {}
		return (sup != nullptr);
	}

	// create rtobject.
	inline eco::rtti::object::ptr create() const
	{
		return m_create();
	}
	static inline eco::rtti::object::ptr create(const char* name)
	{
		return rtti::type_registry::create(name);
	}
	template<typename object_t>
	static inline typename object_t::ptr create()
	{
		return std::dynamic_pointer_cast<object_t>(
			create(object_t::type_name()));
	}
	template<typename object_t>
	static inline typename object_t::ptr create(const char* name)
	{
		return std::dynamic_pointer_cast<object_t>(create(name));
	}

private:
	const char* m_name;
	const eco::rtti::type* m_parent;
	create_func_t m_create;
};


////////////////////////////////////////////////////////////////////////////////
inline bool eco::rtti::object::kind_of(const eco::rtti::object& obj) const
{
	return get_type()->kind_of(obj.get_type());
}
template<typename object_t>
inline bool eco::rtti::object::kind_of() const
{
	return get_type()->kind_of(object_t::type());
}
inline bool eco::rtti::object::same_of(const char* name) const
{
	return eco::equal(get_type()->name(), name);
}
template<typename object_t>
inline bool eco::rtti::object::same_of() const
{
	return get_type() == object_t::type();
}


////////////////////////////////////////////////////////////////////////////////
// runtime object implement.
#define eco_rtti__(object_t)\
public:\
	typedef std::weak_ptr<object_t> wptr;\
	typedef std::shared_ptr<object_t> ptr;\
	inline static const char* type_name()\
	{\
		return eco_macro_str_(object_t);\
	}\
	virtual const eco::rtti::type* get_type() const\
	{\
		return type();\
	}
	
// runtime object who is a instance can be created.
#define eco_rtti(object_t, parent_t)\
eco_rtti__(object_t)\
inline static const eco::rtti::type* type()\
{\
	return &eco::rtti::type_init<object_t, parent_t, &object_t::create>::type;\
}\
inline static eco::rtti::object::ptr create()\
{\
	return std::make_shared<object_t>();\
}
// runtime object who is a interface cann't be created.
#define eco_rtti_i(object_t, parent_t) \
eco_rtti__(object_t) \
inline static const eco::rtti::type* type()\
{\
	return &eco::rtti::type_init<object_t, parent_t, nullptr>::type;\
}\
inline static typename object_t::ptr create(const char* name)\
{\
	return std::dynamic_pointer_cast<object_t>(\
		eco::rtti::type_registry::create(name));\
}


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(rtti);
eco_namespace_end(eco);