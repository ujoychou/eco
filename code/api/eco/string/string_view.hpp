#pragma once
/*******************************************************************************
@ name

@ function

@ exception

@ note

--------------------------------------------------------------------------------
@ [2024-08-24] ujoy created


--------------------------------------------------------------------------------
* copyright(c) 2024 - 2027, ujoy, reserved all right.

*******************************************************************************/
#include <eco/macro.hpp>
#include <string.h>


eco_namespace(eco)
////////////////////////////////////////////////////////////////////////////////
class string_view
{
public:
	template<typename string_t>
	inline string_view(const string_t& v) 
		: data_(v.c_str()), size_(static_cast<uint32_t>(v.size()))
	{}

	inline string_view(const char* v) : data_(v), size_(-1)
	{}

	inline string_view(const char* v, uint32_t size) : data_(v), size_(size)
	{}

	inline bool null() const
	{
		return (data_ == NULL);
	}

	inline const char* c_str() const
	{
		return data_;
	}

	inline const char* c_end() const
	{
		return data_ + size();
	}

	inline char operator[](uint32_t index) const
	{
		return null() ? 0 : data_[index];
	}

	inline uint32_t size() const
	{
		if (size_ == (uint32_t)-1) { size_ = !null() ? strlen(data_) : 0; }
		return size_;
	}

	inline string_view& size(uint32_t v)
	{
		if (v < size()) { size_ = v; }
		return *this;
	}

private:
	const char* data_;
	mutable uint32_t size_;
};


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(eco)