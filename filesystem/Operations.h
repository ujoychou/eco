#ifndef ECO_FILESYSTEM_OPERATIONS_H
#define ECO_FILESYSTEM_OPERATIONS_H
/*******************************************************************************
@ name

@ function


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2015-01-15.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2015 - 2017, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Export.h>
#include <eco/filesystem/File.h>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string.hpp>




namespace eco{;
namespace filesystem{;
////////////////////////////////////////////////////////////////////////////////
enum
{
	remove_root			= 0x01,		// remove root directory
	remove_recursive	= 0x02,		// remove directory recursively.
};
typedef int Option;

////////////////////////////////////////////////////////////////////////////////
// shim class for read file
class ReadFile : public boost::noncopyable
{
public:
	inline ReadFile()
	{}

	inline ReadFile(
		IN const std::string& file,
		IN const char* mode)
	{
		open(file, mode);
	}

	inline void open(
		IN const std::string& file,
		IN const char* mode)
	{
		eco::filesystem::FileRaw reader;
		reader.open(file.c_str(), mode);
		uint32_t size = (uint32_t)reader.file_size(file.c_str());
		if (size > 0)
		{
			m_file_data.resize(size);
			reader.read(&m_file_data[0], size);
		}
	}

	inline std::string& data()
	{
		return m_file_data;
	}
	inline const std::string& get_data() const
	{
		return m_file_data;
	}

private:
	std::string m_file_data;
};


////////////////////////////////////////////////////////////////////////////////
/*@ recursively copy all file in the directory.*/
inline void copy_files(
	IN  const boost::filesystem::path& sour_dir, 
	IN  const boost::filesystem::path& dest_dir)
{
	using namespace boost::filesystem;

	// create destination directory.
	if (!exists(dest_dir))
	{
		create_directories(dest_dir);
	}

	// recursively copy files.
	for (directory_iterator it(sour_dir); it!=directory_iterator(); ++it)
	{
		const path& it_sour = it->path();
		const path it_dest = dest_dir / it->path().filename();
		if (boost::filesystem::is_directory(it_sour))
		{
			eco::filesystem::copy_files(it_sour, it_dest);
		}
		else if (boost::filesystem::is_regular_file(it_sour))
		{
			copy_file(it_sour, it_dest, copy_option::overwrite_if_exists);
		}
	}// end for.
}


////////////////////////////////////////////////////////////////////////////////
/*@ recursively remove all file in the directory.*/
inline bool remove_files_if_impl(
	IN  const boost::filesystem::path& sour_dir,
	IN  const Option option,
	IN  std::function<bool(const boost::filesystem::path& file)>& remove_if)
{
	using namespace boost::filesystem;

	// recursively remove files.
	bool remove_root_sour = true;
	for (directory_iterator it(sour_dir); it != directory_iterator(); ++it)
	{
		const path& sour = it->path();
		if (boost::filesystem::is_directory(sour) && 
			eco::has(option, remove_recursive))
		{
			if (!remove_files_if_impl(sour, remove_root | option, remove_if))
			{
				remove_root_sour = false;
			}
		}
		else if (boost::filesystem::is_regular_file(sour))
		{
			if (!remove_if || remove_if(sour))
			{
				boost::filesystem::remove(sour);
				continue;
			}
			remove_root_sour = false;
		}
	}// end for.

	if (remove_root_sour && eco::has(option, remove_root))
	{
		boost::filesystem::remove(sour_dir);
	}
	return remove_root_sour;
}
inline bool remove_files_if(
	IN  const boost::filesystem::path& sour_dir,
	IN  const Option option,
	IN  std::function<bool(const boost::filesystem::path& file)> remove_if)
{
	return remove_files_if_impl(sour_dir, option, remove_if);
}


////////////////////////////////////////////////////////////////////////////////
/*@ recursively remove all file in the directory.*/
inline void remove_files(
	IN  const boost::filesystem::path& sour_dir,
	IN  const bool remove_sour_dir)
{
	if (exists(sour_dir))
	{
		eco::filesystem::remove_files_if(sour_dir, remove_recursive, nullptr);
	}
}

////////////////////////////////////////////////////////////////////////////////
/*@ recursively remove all file in the directory.
suffix: .log|.txt|.doc
*/
inline void remove_files_by_suffix(
	IN  const boost::filesystem::path& sour_dir,
	IN  const std::string& suffix)
{
	if (exists(sour_dir))
	{
		eco::filesystem::remove_files_if(sour_dir, 0,
			[&](const boost::filesystem::path& file)-> bool {
			return suffix.find(file.extension().string()) != -1;
		});
	}
}


////////////////////////////////////////////////////////////////////////////////
inline void filepath_format(OUT std::string& p)
{
	std::replace(p.begin(), p.end(), '/', '\\');

	// remove "../"
	std::string new_p;
	new_p.reserve(p.size());
	size_t pos = 0;
	while (pos < p.size() - 1)
	{
		if (p[pos] == '.' && p[pos + 1] == '.')			// parent dir.
		{
			// remove parent dir.
			if (new_p.size() > 1 && new_p.back() == '\\')
			{
				for (size_t i = new_p.size() - 2; i != size_t(-1); --i)
				{
					if (new_p[i] == '\\')
					{
						new_p.resize(i);
						break;
					}
				}
			}// end remove.
			pos += 2;
		}
		else if (p[pos] == '.' && p[pos + 1] == '\\')		// current dir.
		{
			pos += 2;
		}
		// igore repeated '\\'
		else if (p[pos] == '\\' && new_p.size() > 0 && new_p.back() == '\\')
		{
			pos += 1;
		}
		else
		{
			new_p.push_back(p[pos]);
			pos += 1;
		}
	}// end for.

	if (pos == p.size() - 1)
	{
		new_p.push_back(p[pos]);
	}

	p = new_p;
}


////////////////////////////////////////////////////////////////////////////////
inline bool filepath_equal(IN const std::string& p1, IN const std::string& p2)
{
	std::string cmp1(p1);
	std::string cmp2(p2);
	filepath_format(cmp1);
	filepath_format(cmp2);
	return boost::algorithm::iequals(cmp1, cmp2);
}


////////////////////////////////////////////////////////////////////////////////
}// ns::filesystem
}// ns::afw
#endif