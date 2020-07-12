#ifndef ECO_IMPLEMENT_H
#define ECO_IMPLEMENT_H
/*******************************************************************************
@ name
dll export template implement.

@ function
1.object api implement.
2.value api implement.
3.property implement.
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
#include <eco/Export.h>
#include <vector>
#include <algorithm>


////////////////////////////////////////////////////////////////////////////////
#define ECO_IMPL_impl(type_t)\
type_t::Impl& type_t::impl()\
{\
	return (type_t::Impl&)(*m_impl);\
}\
const type_t::Impl& type_t::impl() const\
{\
	return (const type_t::Impl&)(*m_impl);\
}
#define ECO_IMPL_default(type_t, impl_t)\
type_t::type_t()\
{\
	m_impl = new impl_t;\
	m_impl->init(*this);\
}
#define ECO_IMPL_derive(type_t, impl_t, parent_t)\
type_t::type_t() : parent_t(eco::null)\
{\
	m_impl = new impl_t; \
	m_impl->init(*this); \
}
#define ECO_IMPL_destruct(type_t)\
type_t::~type_t()\
{\
	delete m_impl;\
	m_impl = nullptr;\
}
#define ECO_IMPL_nullheap(type_t, impl_t)\
type_t::type_t(IN eco::Null)\
{\
	m_impl = nullptr;\
}\
type_t::type_t(IN type_t&& val)\
{\
	m_impl = val.m_impl;\
	val.m_impl = nullptr;\
}\
type_t& type_t::operator=(IN type_t&& val)\
{\
	reset();\
	m_impl = val.m_impl;\
	val.m_impl = nullptr;\
	return *this;\
}\
type_t& type_t::operator=(IN eco::Null)\
{\
	reset();\
	return *this;\
}\
type_t& type_t::operator=(IN eco::Heap)\
{\
	m_impl = new impl_t;\
	m_impl->init(*this);\
	return *this;\
}\
bool type_t::null() const\
{\
	return m_impl == nullptr;\
}\
void type_t::swap(IN type_t& val)\
{\
	std::swap(m_impl, val.m_impl); \
}\
void type_t::reserve()\
{\
	if (null()) (*this) = eco::heap; \
}\
void type_t::reset()\
{\
	delete m_impl;\
	m_impl = nullptr;\
}


////////////////////////////////////////////////////////////////////////////////
#define ECO_SINGLETON_IMPL_1(type_t)\
ECO_IMPL_impl(type_t)\
ECO_IMPL_default(type_t, Impl)\
ECO_IMPL_destruct(type_t)

#define ECO_SINGLETON_IMPL_2(type_t, parent_t)\
ECO_IMPL_impl(type_t)\
ECO_IMPL_derive(type_t, Impl, parent_t)\
ECO_IMPL_destruct(type_t, Impl)

#define ECO_SINGLETON_IMPL(...) ECO_MACRO(ECO_SINGLETON_IMPL_, __VA_ARGS__)

////////////////////////////////////////////////////////////////////////////////
#define ECO_TYPE_IMPL_A(type_t)\
ECO_IMPL_impl(type_t)\
ECO_IMPL_destruct(type_t)\
ECO_IMPL_nullheap(type_t, Impl)

#define ECO_TYPE_IMPL_1(type_t)\
ECO_TYPE_IMPL_A(type_t)\
ECO_IMPL_default(type_t, Impl)

#define ECO_TYPE_IMPL_2(type_t, parent_t)\
ECO_TYPE_IMPL_A(type_t)\
ECO_IMPL_derive(type_t, Impl, parent_t)

#define ECO_TYPE_IMPL(...) ECO_MACRO(ECO_TYPE_IMPL_, __VA_ARGS__)

////////////////////////////////////////////////////////////////////////////////
#define ECO_VALUE_IMPL_A(type_t)\
type_t::type_t(IN const type_t& val)\
{\
	m_impl = new Impl(val.impl());\
}\
type_t& type_t::operator=(IN const type_t& val)\
{\
	impl() = val.impl();\
	return *this;\
}

#define ECO_VALUE_IMPL_1(type_t) \
ECO_TYPE_IMPL_1(type_t) \
ECO_VALUE_IMPL_A(type_t)

#define ECO_VALUE_IMPL_2(type_t, parent_t) \
ECO_TYPE_IMPL_2(type_t, parent_t) \
ECO_VALUE_IMPL_A(type_t)

#define ECO_VALUE_IMPL(...) ECO_MACRO(ECO_VALUE_IMPL_, __VA_ARGS__)


////////////////////////////////////////////////////////////////////////////////
#define ECO_OBJECT_IMPL_1(type_t)\
ECO_TYPE_IMPL_1(type_t)

#define ECO_OBJECT_IMPL_2(type_t, parent_t)\
ECO_TYPE_IMPL_2(type_t, parent_t)

#define ECO_OBJECT_IMPL(...) ECO_MACRO(ECO_OBJECT_IMPL_,__VA_ARGS__)

#define ECO_OBJECT_COPY_IMPL(type_t) \
ECO_OBJECT_IMPL(type_t);\
type_t* type_t::copy() const\
{\
	type_t* obj = new type_t(eco::null);\
	obj->m_impl = new Impl(*m_impl);\
	obj->m_impl->init(*this);\
	return obj;\
}


////////////////////////////////////////////////////////////////////////////////
#define ECO_SHARED_PROXY(type_t) \
class type_t::Proxy\
{\
public:\
	inline Proxy(IN bool heap = true)\
	{\
		if (heap) m_impl.reset(new type_t::Impl);\
	}\
	inline operator type_t::Impl&()\
	{\
		return *m_impl;\
	}\
	inline operator const type_t::Impl&() const\
	{\
		return *m_impl;\
	}\
	inline void init(type_t& parent) const\
	{\
		m_impl->init(parent);\
	}\
	std::shared_ptr<type_t::Impl> m_impl;\
};

////////////////////////////////////////////////////////////////////////////////
#define ECO_SHARED_IMPL__(type_t)\
ECO_IMPL_impl(type_t)\
ECO_IMPL_default(type_t, Proxy)\
ECO_IMPL_destruct(type_t)\
ECO_IMPL_nullheap(type_t, Proxy)\
type_t::type_t(IN const type_t& obj) : m_impl(nullptr)\
{\
	if (!obj.null())\
	{\
		m_impl = new Proxy(false);\
		m_impl->m_impl = obj.m_impl->m_impl;\
	}\
}\
type_t& type_t::operator=(IN const type_t& obj)\
{\
	if (obj.null()) { reset(); return *this; }\
	if (null()) m_impl = new Proxy(false); \
	m_impl->m_impl = obj.m_impl->m_impl; \
	return *this;\
}\
bool type_t::operator==(IN const type_t& obj) const\
{\
	if (null() && obj.null()) return true;\
	if (null() || obj.null()) return false;\
	return m_impl->m_impl == obj.m_impl->m_impl;\
}\
bool type_t::operator!=(IN const type_t& obj) const\
{\
	return !(operator==(obj));\
}
#define ECO_SHARED_COPY_IMPL__(type_t) \
ECO_SHARED_IMPL__(type_t);\
type_t type_t::copy() const\
{\
	type_t obj(eco::null);\
	obj.m_impl = new Proxy(false);\
	obj.m_impl->m_impl.reset(new Impl(*m_impl->m_impl));\
	obj.m_impl->m_impl->init(obj);\
	return obj;\
}

#define ECO_SHARED_IMPL(type_t)\
ECO_SHARED_PROXY(type_t) \
ECO_SHARED_IMPL__(type_t)
#define ECO_SHARED_COPY_IMPL(type_t) \
ECO_SHARED_PROXY(type_t) \
ECO_SHARED_COPY_IMPL__(type_t);


////////////////////////////////////////////////////////////////////////////////
// export value property implement: reference.
#define ECO_PROPERTY_VAL_IMPL(type_t, property_t, property_name) \
void type_t::set_##property_name(IN const property_t& val)\
{\
	impl().m_##property_name = val;\
}\
type_t& type_t::##property_name(IN const property_t& val) \
{\
	impl().m_##property_name = val;\
	return *this;\
}\
property_t& type_t::##property_name()\
{\
	return impl().m_##property_name;\
}\
const property_t& type_t::get_##property_name() const\
{\
	return impl().m_##property_name;\
}
// export value property implement: const value.
#define ECO_PROPERTY_VVC_IMPL(type_t, property_t, property_name) \
void type_t::set_##property_name(IN const property_t val)\
{\
	impl().m_##property_name = val;\
}\
type_t& type_t::##property_name(IN const property_t val) \
{\
	impl().m_##property_name = val;\
	return *this;\
}\
const property_t type_t::get_##property_name() const\
{\
	return impl().m_##property_name;\
}
// export value property implement: value.
#define ECO_PROPERTY_VAV_IMPL(type_t, property_t, property_name) \
ECO_PROPERTY_VVC_IMPL(type_t, property_t, property_name)\
property_t type_t::##property_name()\
{\
	return impl().m_##property_name;\
}
// export object property implement
#define ECO_PROPERTY_OBJ_IMPL_NOHAS(type_t, property_type, property_name)\
void type_t::set_##property_name(IN property_type& v)\
{\
	impl().m_##property_name = v;\
}\
property_type& type_t::##property_name()\
{\
	if (impl().m_##property_name.null())\
		impl().m_##property_name = eco::heap;\
	return impl().m_##property_name;\
}\
const property_type& type_t::get_##property_name() const\
{\
	return impl().m_##property_name;\
}
// export object property implement
#define ECO_PROPERTY_OBJ_IMPL(type_t, property_type, property_name)\
bool type_t::has_##property_name() const\
{\
	return !impl().m_##property_name.null();\
}\
ECO_PROPERTY_OBJ_IMPL_NOHAS(type_t, property_type, property_name);
// export string property implement
#define ECO_PROPERTY_STR_IMPL(type_t, property_name) \
void type_t::set_##property_name(IN const char* val) \
{\
	impl().m_##property_name = val;\
}\
type_t& type_t::##property_name(IN const char* val) \
{\
	impl().m_##property_name = val;\
	return *this;\
}\
const char* type_t::get_##property_name() const\
{\
	return impl().m_##property_name.c_str();\
}
#define ECO_PROPERTY_BUF_IMPL(type_t, property_name) \
void type_t::set_##property_name(IN const char* val) \
{\
	eco_cpyc(impl().m_##property_name, val);\
}\
type_t& type_t::##property_name(IN const char* val) \
{\
	eco_cpyc(impl().m_##property_name, val);\
	return *this;\
}\
const char* type_t::get_##property_name() const\
{\
	return impl().m_##property_name;\
}
// export bool property implement
#define ECO_PROPERTY_BOL_IMPL(type_t, property_name) \
void type_t::set_##property_name(IN const bool val) \
{\
	impl().m_##property_name = val;\
}\
type_t& type_t::##property_name(IN const bool val) \
{\
	impl().m_##property_name = val;\
	return *this;\
}\
bool type_t::##property_name() const\
{\
	return impl().m_##property_name > 0;\
}


////////////////////////////////////////////////////////////////////////////////
// export property set implement.
#define ECO_PROPERTY_SET_IMPL(data_set_t, data_t) \
void data_set_t::add(const data_t& it)\
{\
	impl().m_items.push_back(it);\
}\
void data_set_t::push_back(const data_t& it)\
{\
	impl().m_items.push_back(it);\
}\
void data_set_t::pop_back()\
{\
	impl().m_items.pop_back();\
}\
data_t& data_set_t::add()\
{\
	impl().m_items.push_back(data_t());\
	return impl().m_items.back();\
}\
void data_set_t::add_move(IN data_set_t& v)\
{\
	auto it = v.impl().m_items.begin();\
	for (; it != v.impl().m_items.end(); ++it)\
	{\
		impl().m_items.push_back(std::move(*it));\
	}\
}\
void data_set_t::add_copy(IN const data_set_t& v)\
{\
	auto it = v.impl().m_items.begin();\
	for (; it != v.impl().m_items.end(); ++it)\
	{\
		impl().m_items.push_back(*it);\
	}\
}\
size_t data_set_t::size() const\
{\
	return impl().m_items.size();\
}\
bool data_set_t::empty() const\
{\
	return impl().m_items.size() == 0;\
}\
void data_set_t::reserve(IN const size_t v)\
{\
	return impl().m_items.reserve(v);\
}\
data_t& data_set_t::at(IN int i)\
{\
	return impl().m_items.at(i);\
}\
const data_t& data_set_t::at(IN int i) const\
{\
	return impl().m_items.at(i);\
}\
void data_set_t::erase(IN int i)\
{\
	impl().m_items.erase(impl().m_items.begin() + i);\
}\
data_set_t::iterator data_set_t::erase(IN iterator& it)\
{\
	auto itt = impl().m_items.erase(\
		impl().m_items.begin() + (it - begin()));\
	return (impl().m_items.end() == itt) ? end() : iterator(&*itt);\
}\
void data_set_t::clear()\
{\
	impl().m_items.clear();\
}\
data_set_t::iterator data_set_t::begin()\
{\
	data_t* ptr = nullptr;\
	if (size() > 0)\
	{\
		ptr = &impl().m_items.front();\
	}\
	return iterator(ptr);\
}\
data_set_t::const_iterator data_set_t::begin() const\
{\
	const data_t* ptr = nullptr;\
	if (size() > 0)\
	{\
		ptr = &impl().m_items.front();\
	}\
	return const_iterator(ptr);\
}\
data_set_t::iterator data_set_t::end()\
{\
	data_t* ptr = nullptr;\
	if (size() > 0)\
	{\
		ptr = &impl().m_items.back();\
		++ptr;\
	}\
	return iterator(ptr);\
}\
data_set_t::const_iterator data_set_t::end() const\
{\
	const data_t* ptr = nullptr;\
	if (size() > 0)\
	{\
		ptr = &impl().m_items.back();\
		++ptr;\
	}\
	return const_iterator(ptr);\
}


////////////////////////////////////////////////////////////////////////////////
#endif