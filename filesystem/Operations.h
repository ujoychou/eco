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


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(filesystem);
////////////////////////////////////////////////////////////////////////////////
enum
{
	remove_root			= 0x01,		// remove root directory
	remove_recursive	= 0x02,		// remove directory recursively.
};
typedef int Option;


////////////////////////////////////////////////////////////////////////////////
/*@ check file is exist.*/
inline bool exist_file(IN const char* file)
{
	FILE* fp = ::fopen(file, "r");
	if (fp) { ::fclose(fp); return true; }
	return false;
}


////////////////////////////////////////////////////////////////////////////////
/*@ recursively remove all file in the directory.*/
inline void get_files_if_impl(
	OUT std::vector<boost::filesystem::path>& files,
	IN  const boost::filesystem::path& sour_dir,
	IN  const bool recursive,
	IN  const std::function<bool(const boost::filesystem::path& file)>& filter)
{
	using namespace boost::filesystem;

	// recursively remove files.
	for (directory_iterator it(sour_dir); it != directory_iterator(); ++it)
	{
		const path& sour = it->path();
		if (boost::filesystem::is_directory(sour) && recursive)
		{
			get_files_if_impl(files, sour, true, filter);
		}
		else if (boost::filesystem::is_regular_file(sour))
		{
			if (filter && filter(sour))	files.push_back(sour);  
		}
	}// end for.
}


////////////////////////////////////////////////////////////////////////////////
inline void get_files_if(
	OUT std::vector<boost::filesystem::path>& files,
	IN  const boost::filesystem::path& sour_dir,
	IN  const bool recursive,
	IN  const std::function<bool(const boost::filesystem::path& file)>& filter)
{
	if (!exists(sour_dir)) return;
	get_files_if_impl(files, sour_dir, recursive, filter);
}


////////////////////////////////////////////////////////////////////////////////
/*@ recursively copy all file in the directory.*/
inline void copy_files_impl(
	IN  const boost::filesystem::path& sour_dir,
	IN  const boost::filesystem::path& dest_dir,
	IN  const bool recursive)
{
	using namespace boost::filesystem;

	// recursively copy files.
	for (directory_iterator it(sour_dir); it != directory_iterator(); ++it)
	{
		const path& it_sour = it->path();
		const path it_dest = dest_dir / it->path().filename();
		if (boost::filesystem::is_directory(it_sour) && recursive)
		{
			// create destination directory.
			if (!exists(it_dest)) create_directory(it_dest);

			// copy destination files.
			eco::filesystem::copy_files_impl(it_sour, it_dest, recursive);
		}
		else if (boost::filesystem::is_regular_file(it_sour))
		{
			copy_file(it_sour, it_dest, copy_option::overwrite_if_exists);
		}
	}// end for.
}


////////////////////////////////////////////////////////////////////////////////
/*@ recursively copy all file in the directory.*/
inline void copy_files(
	IN  const boost::filesystem::path& sour_dir, 
	IN  const boost::filesystem::path& dest_dir,
	IN  const bool recursive = true)
{
	using namespace boost::filesystem;

	// create destination directory.
	if (!exists(dest_dir))
	{
		create_directories(dest_dir);
	}
	copy_files_impl(sour_dir, dest_dir, recursive);
}


////////////////////////////////////////////////////////////////////////////////
/*@ recursively remove all file in the directory.*/
inline bool remove_files_if_impl(
	IN  const boost::filesystem::path& sour_dir,
	IN  const Option option,
	IN  const std::function<bool(const boost::filesystem::path& file)>& filter)
{
	using namespace boost::filesystem;

	// recursively remove files.
	bool remove_root_sour = true;
	for (directory_iterator it(sour_dir); it != directory_iterator(); ++it)
	{
		const path& sour = it->path();
		if (is_directory(sour) && eco::has(option, remove_recursive))
		{
			if (!remove_files_if_impl(sour, remove_root | option, filter))
				remove_root_sour = false;
		}
		else if (boost::filesystem::is_regular_file(sour))
		{
			if (!filter || filter(sour))
				boost::filesystem::remove(sour);
			else
				remove_root_sour = false;
		}
	}// end for.

	if (remove_root_sour && eco::has(option, remove_root))
		boost::filesystem::remove(sour_dir);
	return remove_root_sour;
}
inline bool remove_files_if(
	IN  const boost::filesystem::path& sour_dir,
	IN  const Option option,
	IN  const std::function<bool(const boost::filesystem::path& file)>& filter)
{
	if (!exists(sour_dir)) return false;
	return remove_files_if_impl(sour_dir, option, filter);
}


////////////////////////////////////////////////////////////////////////////////
/*@ recursively remove all file in the directory.*/
inline void remove_files(IN  const boost::filesystem::path& sour_dir)
{
	eco::filesystem::remove_files_if(sour_dir, remove_recursive, nullptr);
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
// write string to files.
inline void write_file(const char* file, const char* c, size_t size)
{
	eco::filesystem::File f(file, "wt+");
	f.write(c, size);
	f.close();
}
inline void write_file(const char* file, const std::string& c)
{
	write_file(file, c.c_str(), c.size());
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(filesystem);
ECO_NS_END(eco);
#endif