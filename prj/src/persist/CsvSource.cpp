#include "Pch.h"
#include <eco/persist/CsvSource.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/filesystem/File.h>
#include <eco/sys/Win.h>


namespace eco {;

////////////////////////////////////////////////////////////////////////////////
class CsvSourceFile::Impl
{
public:
	std::shared_ptr<eco::filesystem::File> m_csv;

	void init(CsvSourceFile&){}
};
ECO_SHARED_IMPL(CsvSourceFile);


////////////////////////////////////////////////////////////////////////////////
void CsvSourceFile::open(IN const char* csv_file, IN const char* mode)
{
	impl().m_csv.reset(new eco::filesystem::File(csv_file, mode));
}


////////////////////////////////////////////////////////////////////////////////
void CsvSourceFile::write(IN const char* data, IN int size)
{
	impl().m_csv->write(data, size);
}


////////////////////////////////////////////////////////////////////////////////
void CsvSourceFile::read(OUT std::string& data)
{
	size_t size = (size_t)impl().m_csv->file_size();
	data.resize(size);
	impl().m_csv->read(&data[0], size);
}


}
