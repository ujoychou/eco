#ifndef ECO_EXPORT_IMPL_H
#define ECO_EXPORT_IMPL_H
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
#define ECO_IMPL(type_t)\
type_t::Impl& type_t::impl()\
{\
	return (type_t::Impl&)(*m_impl);\
}\
const type_t::Impl& type_t::impl() const\
{\
	return (const type_t::Impl&)(*m_impl);\
}

////////////////////////////////////////////////////////////////////////////////
#define ECO_TYPE_IMPL(type_t)\
type_t::type_t()\
{\
	m_impl = new Impl;\
	m_impl->init(*this);\
}\
type_t::~type_t()\
{\
	delete m_impl;\
	m_impl = nullptr;\
}\
ECO_IMPL(type_t)


////////////////////////////////////////////////////////////////////////////////
#define ECO_VALUE_IMPL(value_t) \
ECO_TYPE_IMPL(value_t);\
value_t::value_t(IN const value_t& val)\
{\
	m_impl = new Impl(val.impl());\
}\
value_t& value_t::operator=(IN const value_t& val)\
{\
	impl() = val.impl();\
	return *this;\
}


////////////////////////////////////////////////////////////////////////////////
#define ECO_OBJECT_IMPL(object_t) \
ECO_TYPE_IMPL(object_t);\
object_t::object_t(IN eco::Null)\
{\
	m_impl = nullptr;\
}\
object_t& object_t::operator=(IN eco::Null)\
{\
	reset();\
	return *this;\
}\
object_t& object_t::operator=(IN eco::Heap)\
{\
	m_impl = new Impl;\
	m_impl->init(*this);\
	return *this;\
}\
bool object_t::null() const\
{\
	return m_impl == nullptr;\
}\
void object_t::reset()\
{\
	delete m_impl;\
	m_impl = nullptr;\
}


////////////////////////////////////////////////////////////////////////////////
#define ECO_MOVABLE_IMPL(object_t) \
ECO_OBJECT_IMPL(object_t);\
object_t::object_t(IN object_t&& obj) : m_impl(obj.m_impl)\
{\
	obj.reset();\
}\
object_t& object_t::operator=(IN object_t&& obj)\
{\
	reset();\
	m_impl = obj.m_impl;\
	obj.reset();\
	return *this;\
}


////////////////////////////////////////////////////////////////////////////////
#define ECO_SHARED_PROXY(object_t) \
class object_t::Proxy\
{\
public:\
	inline Proxy(IN bool heap)\
	{\
		if (heap)\
			m_impl.reset(new object_t::Impl);\
	}\
	inline object_t::Impl& impl() const\
	{\
		return *m_impl;\
	}\
	std::shared_ptr<object_t::Impl> m_impl;\
};

#define ECO_SHARED_IMPL__(object_t)\
object_t::Impl& object_t::impl() const\
{\
	return m_proxy->impl();\
}\
object_t::object_t()\
{\
	m_proxy = new Proxy(true);\
	m_proxy->m_impl->init(*this);\
}\
object_t::~object_t()\
{\
	delete m_proxy;\
	m_proxy = nullptr;\
}\
object_t::object_t(IN eco::Null)\
{\
	m_proxy = new Proxy(false);\
}\
object_t::object_t(IN object_t&& obj)\
{\
	m_proxy = new Proxy(false);\
	m_proxy->m_impl = std::move(obj.m_proxy->m_impl);\
}\
object_t::object_t(IN const object_t& obj)\
{\
	m_proxy = new Proxy(false);\
	m_proxy->m_impl = obj.m_proxy->m_impl;\
}\
object_t& object_t::operator=(IN object_t&& obj)\
{\
	m_proxy->m_impl = std::move(obj.m_proxy->m_impl);\
	return *this;\
}\
object_t& object_t::operator=(IN const object_t& obj)\
{\
	m_proxy->m_impl = obj.m_proxy->m_impl;\
	return *this;\
}\
object_t& object_t::operator=(IN eco::Null)\
{\
	reset();\
	return *this;\
}\
object_t& object_t::operator=(IN eco::Heap)\
{\
	m_proxy->m_impl.reset(new Impl);\
	m_proxy->m_impl->init(*this);\
	return *this;\
}\
bool object_t::operator==(IN const object_t& obj) const\
{\
	return m_proxy->m_impl == obj.m_proxy->m_impl;\
}\
bool object_t::operator!=(IN const object_t& obj) const\
{\
	return m_proxy->m_impl != obj.m_proxy->m_impl;\
}\
bool object_t::null() const\
{\
	return (m_proxy->m_impl.get() == nullptr);\
}\
void object_t::swap(IN object_t& obj)\
{\
	m_proxy->m_impl.swap(obj.m_proxy->m_impl);\
}\
void object_t::reset()\
{\
	m_proxy->m_impl.reset();\
}\
void object_t::reserve()\
{\
	if ((m_proxy->m_impl.get() == nullptr)) {\
		m_proxy->m_impl.reset(new Impl);\
		m_proxy->m_impl->init(*this);\
	}\
}

#define ECO_SHARED_IMPL(object_t)\
ECO_SHARED_PROXY(object_t) \
ECO_SHARED_IMPL__(object_t)


////////////////////////////////////////////////////////////////////////////////
// export value property implement
#define ECO_PROPERTY_ATM_IMPL(object_t, property_t, property_name) \
void object_t::set_##property_name(IN const property_t val)\
{\
	impl().m_##property_name = val;\
}\
object_t& object_t::##property_name(IN const property_t val) \
{\
	impl().m_##property_name = val;\
	return *this;\
}\
const property_t object_t::get_##property_name() const\
{\
	return impl().m_##property_name;\
}
// export value property implement
#define ECO_PROPERTY_VAL_IMPL(object_t, property_t, property_name) \
void object_t::set_##property_name(IN const property_t& val)\
{\
	impl().m_##property_name = val;\
}\
object_t& object_t::##property_name(IN const property_t& val) \
{\
	impl().m_##property_name = val;\
	return *this;\
}\
property_t& object_t::##property_name()\
{\
	return impl().m_##property_name;\
}\
const property_t& object_t::get_##property_name() const\
{\
	return impl().m_##property_name;\
}
// export value property implement
#define ECO_PROPERTY_VAV_IMPL(object_t, property_t, property_name) \
void object_t::set_##property_name(IN const property_t val)\
{\
	impl().m_##property_name = val;\
}\
object_t& object_t::##property_name(IN const property_t val) \
{\
	impl().m_##property_name = val;\
	return *this;\
}\
property_t object_t::##property_name()\
{\
	return impl().m_##property_name;\
}\
const property_t object_t::get_##property_name() const\
{\
	return impl().m_##property_name;\
}
// export object property implement
#define ECO_PROPERTY_OBJ_IMPL_NOHAS(object_t, property_type, property_name)\
void object_t::set_##property_name(IN property_type& v)\
{\
	impl().m_##property_name = v;\
}\
property_type& object_t::##property_name()\
{\
	if (impl().m_##property_name.null())\
		impl().m_##property_name = eco::heap;\
	return impl().m_##property_name;\
}\
const property_type& object_t::get_##property_name() const\
{\
	return impl().m_##property_name;\
}
// export object property implement
#define ECO_PROPERTY_OBJ_IMPL(object_t, property_type, property_name)\
bool object_t::has_##property_name() const\
{\
	return !impl().m_##property_name.null();\
}\
ECO_PROPERTY_OBJ_IMPL_NOHAS(object_t, property_type, property_name);
// export string property implement
#define ECO_PROPERTY_STR_IMPL(object_t, property_name) \
void object_t::set_##property_name(IN const char* val) \
{\
	impl().m_##property_name = val;\
}\
object_t& object_t::##property_name(IN const char* val) \
{\
	impl().m_##property_name = val;\
	return *this;\
}\
const char* object_t::get_##property_name() const\
{\
	return impl().m_##property_name.c_str();\
}
#define ECO_PROPERTY_BUF_IMPL(object_t, property_name) \
void object_t::set_##property_name(IN const char* val) \
{\
	eco_cpyc(impl().m_##property_name, val);\
}\
object_t& object_t::##property_name(IN const char* val) \
{\
	eco_cpyc(impl().m_##property_name, val);\
	return *this;\
}\
const char* object_t::get_##property_name() const\
{\
	return impl().m_##property_name;\
}
// export bool property implement
#define ECO_PROPERTY_BOL_IMPL(object_t, property_name) \
void object_t::set_##property_name(IN const bool val) \
{\
	impl().m_##property_name = val;\
}\
object_t& object_t::##property_name(IN const bool val) \
{\
	impl().m_##property_name = val;\
	return *this;\
}\
bool object_t::##property_name() const\
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