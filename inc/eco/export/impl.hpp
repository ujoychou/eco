#pragma once
/*******************************************************************************
@ name
dll export template implement.

@ function
1.value api implement.
2.object api implement.
3.shared api implement.
4.property implement.
4.property set implement.

@ exception

@ note

--------------------------------------------------------------------------------
@ [history ver 1.0]
@ ujoy modifyed on 2016-05-06.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2017, ujoy, reserved all right.

*******************************************************************************/
#include <eco/export/api.hpp>
#include <atomic>
#include <malloc.h>


eco_namespace(eco);
////////////////////////////////////////////////////////////////////////////////
template<typename impl_t, typename type_t>
inline impl_t* impl_init(type_t& obj)
{
	char*  buff = static_cast<char*>(malloc(sizeof(impl_t)));
	new (buff) impl_t;
	impl_t* impl = static_cast<impl_t*>(buff);
	//impl->init(obj);
	return impl;
}

template<typename impl_t, typename type_t>
inline impl_t* impl_copy(type_t& obj, const type_t& value)
{
	if (value.null())
	{
		char*  buff = static_cast<char*>(malloc(sizeof(impl_t)));
		new (buff) impl_t(*value.m_impl);
		impl_t* impl = reinterpret_cast<impl_t*>(buff);
		//impl->init(obj);
		return impl;
	}
	return nullptr;
}

template<typename impl_t>
inline void impl_reset(impl_t*& impl)
{
	if (impl != nullptr)
	{
		impl->~impl_t();
		free(impl);
		impl = nullptr;
	}
}


////////////////////////////////////////////////////////////////////////////////
template<typename impl_t, typename type_t>
inline impl_t* impl_shared_init(type_t& obj, int ref)
{
	size_t size = sizeof(impl_t) + sizeof(std::atomic<int>);
	char*  buff = static_cast<char*>(malloc(size));
	new (buff) impl_t;
	new (buff + sizeof(impl_t)) std::atomic<int>(ref);
	impl_t* impl = reinterpret_cast<impl_t*>(buff);
	//impl->init(obj);
	return impl;
}

template<typename impl_t>
inline int impl_shared_ref(impl_t* impl)
{
	auto* pref = reinterpret_cast<std::atomic<int>*>(impl + sizeof(impl_t));
	return ++(*pref);
}

template<typename impl_t>
inline void impl_shared_reset(impl_t*& impl)
{
	auto* pref = reinterpret_cast<std::atomic<int>*>(impl + sizeof(impl_t));
	if (pref->fetch_sub(1) == 1)
	{
		impl->~impl_t();
		free(impl);
		impl = nullptr;
	}
}


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(eco);


////////////////////////////////////////////////////////////////////////////////
#define eco_object_impl(type_t, impl_t)\
type_t::type_t() {}\
type_t::type_t(eco::heap_t)\
{\
	m_impl = eco::impl_init<impl_t>(*this);\
}\
type_t::type_t(type_t&& val)\
{\
	m_impl = val.m_impl;\
	val.m_impl = nullptr;\
}\
type_t::~type_t()\
{\
	eco::impl_reset(m_impl);\
}\
type_t& type_t::operator=(type_t&& val)\
{\
	eco::impl_reset(m_impl);\
	m_impl = val.m_impl;\
	val.m_impl = nullptr;\
	return *this;\
}\
type_t& type_t::operator=(eco::heap_t)\
{\
	eco::impl_reset(m_impl);\
	m_impl = eco::impl_init<impl_t>(*this);\
	return *this;\
}\
bool type_t::null() const\
{\
	return m_impl == nullptr;\
}\
void type_t::reset()\
{\
	eco::impl_reset(m_impl);\
}


////////////////////////////////////////////////////////////////////////////////
#define eco_shared_impl(type_t, impl_t)\
type_t::type_t() {}\
type_t::type_t(eco::heap_t)\
{\
	m_impl = eco::impl_shared_init<impl_t>(*this, 1);\
}\
type_t::type_t(type_t&& val)\
{\
	m_impl = val.m_impl;\
	val.m_impl = nullptr;\
}\
type_t::~type_t()\
{\
	eco::impl_shared_reset(m_impl);\
}\
type_t& type_t::operator=(type_t&& val)\
{\
	eco::impl_shared_reset(m_impl);\
	m_impl = val.m_impl;\
	val.m_impl = nullptr;\
	return *this;\
}\
type_t& type_t::operator=(eco::heap_t)\
{\
	eco::impl_shared_reset(m_impl);\
	m_impl = eco::impl_shared_init<impl_t>(*this, 1);\
	return *this;\
}\
bool type_t::null() const\
{\
	return m_impl == nullptr;\
}\
void type_t::reset()\
{\
	eco::impl_shared_reset(m_impl);\
}\
type_t::type_t(const type_t& val)\
{\
	if (!val.null())\
	{\
		eco::impl_shared_ref(val.m_impl);\
		m_impl = val.m_impl;\
	}\
}\
type_t& type_t::operator=(const type_t& val)\
{\
	eco::impl_shared_reset(m_impl);\
	if (!val.null())\
	{\
		eco::impl_shared_ref(val.m_impl);\
		m_impl = val.m_impl;\
	}\
	return *this;\
}


////////////////////////////////////////////////////////////////////////////////
#define eco_value_impl(type_t, impl_t)\
eco_object_impl(type_t, impl_t)\
type_t::type_t(const type_t& val)\
{\
	m_impl = eco::impl_copy<impl_t>(*this, *val.m_impl);\
}\
type_t& type_t::operator=(const type_t& val)\
{\
	if (m_impl) { *m_impl = *val.m_impl; return *this; }\
	m_impl = eco::impl_copy<impl_t>(*this, val);\
	return *this;\
}



////////////////////////////////////////////////////////////////////////////////
// implement string member 
#define eco_member_impl_string(type_t, member_name) \
type_t& type_t::member_name(const char* val) \
{\
	m_impl->member_name = val;\
	return *this;\
}\
const char* type_t::member_name() const\
{\
	return m_impl->member_name.c_str();\
}

// implement value member (int, float, double, etc.)
#define eco_member_impl_value(type_t, value_t, member_name) \
type_t& type_t::member_name(value_t val) \
{\
	m_impl->member_name = val;\
	return *this;\
}\
value_t type_t::member_name() const\
{\
	return m_impl->member_name;\
}

// implement reference member (struct, class, etc.)
#define eco_member_impl_ref(type_t, ref_t, member_name) \
type_t& type_t::member_name(const reference_t& val) \
{\
	m_impl->member_name = val;\
	return *this;\
}\
const reference_t& type_t::member_name() const \
{\
	return m_impl->member_name; \
}\
reference_t& type_t::member_name() \
{\
	return m_impl->m_##member_name;\
}
