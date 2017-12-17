#include "PrecHeader.h"
#include <eco/persist/Recordset.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/Project.h>
#include <vector>


namespace eco{;

//##############################################################################
//##############################################################################
class Record::Impl
{
public:
	std::vector<std::string> m_record;

	void init(Record&){}
};
ECO_SHARED_IMPL(Record);
////////////////////////////////////////////////////////////////////////////////
void Record::reserve(IN const size_t size)
{
	impl().m_record.reserve(size);
}
const size_t Record::size() const
{
	return impl().m_record.size();
}
void Record::add(IN const char* item)
{
	impl().m_record.push_back(item);
}
const char* Record::at(IN const size_t index) const
{
	return impl().m_record.at(index).c_str();
}
const char* Record::operator[](IN const size_t index) const
{
	return impl().m_record.at(index).c_str();
}
void Record::clear()
{
	return impl().m_record.clear();
}


//##############################################################################
//##############################################################################
class Recordset::Impl
{
public:
	std::vector<Record> m_record_set;

	void init(Recordset&){}
};
ECO_SHARED_IMPL(Recordset);
////////////////////////////////////////////////////////////////////////////////
void Recordset::reserve(IN const size_t size)
{
	impl().m_record_set.reserve(size);
}
const size_t Recordset::size() const
{
	return impl().m_record_set.size();
}
Record& Recordset::add_item()
{
	Record item;
	impl().m_record_set.push_back(item);
	return impl().m_record_set.back();
}
Record& Recordset::at(IN const size_t index)
{
	return impl().m_record_set.at(index);
}
const Record& Recordset::at(IN const size_t index) const
{
	return impl().m_record_set.at(index);
}
Record& Recordset::operator[](IN const size_t index)
{
	return impl().m_record_set.at(index);
}
const Record& Recordset::operator[](IN const size_t index) const
{
	return impl().m_record_set.at(index);
}
void Recordset::clear()
{
	return impl().m_record_set.clear();
}


}// ns