#pragma once
/*******************************************************************************
@ name
rx export.

@ function
#.rx export declear.

@ exception

@ note

--------------------------------------------------------------------------------
@ [history ver 1.0]
@ ujoy modifyed on 2016-05-06.
1.create and init this class.

@ ujoy modifyed on 2021-11-20
1.Export.h -> RxExport.h


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2025, ujoy, reserved all right.

*******************************************************************************/
#include <eco/export/api.hpp>


eco_namespace(eco)
////////////////////////////////////////////////////////////////////////////////
struct null_t {};
const null_t null;
struct heap_t {};
const heap_t heap;
////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(eco)


////////////////////////////////////////////////////////////////////////////////
#define eco_object_api(type_t) \
private:\
	class impl;\
	friend class impl;\
	impl* m_impl = nullptr;\
public: \
	type_t(); \
	type_t(type_t&&); \
	type_t(eco::heap_t); \
	~type_t(); \
	type_t& operator=(type_t&&); \
	type_t& operator=(eco::heap_t); \
	bool null() const;\
	void reset();

#define eco_shared_api(type_t) \
eco_object_api(type_t);\
type_t(const type_t&); \
type_t& operator=(const type_t&);

#define eco_value_api(type_t) \
eco_object_api(type_t);\
type_t(const type_t&); \
type_t& operator=(const type_t&);

