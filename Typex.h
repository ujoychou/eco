#ifndef ECO_TYPEX_H
#define ECO_TYPEX_H
/*******************************************************************************
@ name
eco basic type.

@ function
1.string_any.
2.variant.

@ exception


@ note

--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2013-01-01.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2013 - 2015, ujoy, reserved all right.

*******************************************************************************/
#include <eco/ExportApi.h>


namespace eco{;


////////////////////////////////////////////////////////////////////////////////
class ECO_API StringAny
{
	ECO_VALUE_API(StringAny);
public:
	StringAny(IN const char*);
	StringAny& operator=(IN const char*);
	bool operator==(IN const StringAny&) const;
	bool empty() const;
	const char* c_str() const;

	operator const char*() const;
	operator const char() const;
	operator const unsigned char() const;
	operator const short() const;
	operator const unsigned short() const;
	operator const int() const;
	operator const unsigned int() const;
	operator const long() const;
	operator const unsigned long() const;
	operator const int64_t() const;
	operator const uint64_t() const;
	operator const double() const;
	operator const bool() const;
};


////////////////////////////////////////////////////////////////////////////////
class ECO_API Parameter
{
	ECO_VALUE_API(Parameter);
public:
	/*@ key.*/
	void set_key(IN const char* key);
	const char* get_key() const;
	Parameter& key(IN const char* key);

	/*@ value.*/
	void set_value(IN const StringAny& val);
	StringAny& value();
	void set_value(IN const char* val);
	const StringAny& get_value() const;
	Parameter& value(IN const StringAny& val);
	Parameter& value(IN const char* val);

public:
	operator const char*() const;
	operator const char() const;
	operator const unsigned char() const;
	operator const short() const;
	operator const unsigned short() const;
	operator const int() const;
	operator const unsigned int() const;
	operator const long() const;
	operator const unsigned long() const;
	operator const int64_t() const;
	operator const uint64_t() const;
	operator const double() const;
	operator const bool() const;
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
	void reserve(IN const size_t capacity);

	/*@ access parameter by item index.*/
	Parameter& at(IN const int i);
	const Parameter& at(IN const int i) const;

public:
	// whether has a key.
	bool has(IN const char* key) const;

	// find the key value.
	bool find(OUT StringAny& v, IN const char* key) const;

	// get the key value.
	StringAny at(IN const char* key) const;
};


////////////////////////////////////////////////////////////////////////////////
class ContextNodeSet;
class ECO_API ContextNode
{
	ECO_SHARED_API(ContextNode);
public:
	void set_name(IN const char*);
	const char* get_name() const;
	ContextNode& name(IN const char*);

	void set_value(IN const char*);
	const char* get_value() const;
	ContextNode& value(IN const char*);

	void set_property_set(IN const eco::Context&);
	eco::Context& property_set();
	const eco::Context& get_property_set() const;
	ContextNode& property_set(IN const eco::Context&);

	bool has_children() const;
	void set_children(IN eco::ContextNodeSet&);
	eco::ContextNodeSet& children();
	const eco::ContextNodeSet& get_children() const;
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
	void reserve(IN const size_t capacity);

	/*@ access parameter by item index.*/
	ContextNode& at(IN const int i);
	const ContextNode& at(IN const int i) const;
};


////////////////////////////////////////////////////////////////////////////////
class ECO_API StdString
{
	ECO_VALUE_API(StdString);
public:
	StdString(const char*);

	// 字符串值
	void set_value(const char*);
	const char* get_value() const;
	StdString& value(const char*);

	// 重载操作符
	const char* c_str() const;
	operator const char*() const;
};


////////////////////////////////////////////////////////////////////////////////
class ECO_API StdStringSet
{
	ECO_SHARED_API(StdStringSet);
public:
	typedef StdString value_type;
	typedef eco::iterator<StdString> iterator;
	typedef eco::iterator<const StdString> const_iterator;
	iterator begin();
	const_iterator begin() const;
	iterator end();
	const_iterator end() const;

	// 移动拷贝
	void add_move(IN StdStringSet&);
	void add_copy(IN const StdStringSet&);

	// 添加
	StdString& add();
	void add(IN const StdString&);
	void push_back(IN const StdString& v);

	// 删除
	void erase(IN int);
	iterator erase(IN iterator& it);
	void pop_back();

	// 清理
	void clear();

	// 大小
	size_t size() const;
	bool empty() const;
	void reserve(IN const size_t capacity);

	// 通过下标访问数组对象
	StdString& at(IN const int i);
	const StdString& at(IN const int i) const;
};


////////////////////////////////////////////////////////////////////////////////
}//
#endif