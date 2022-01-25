#ifndef ECO_FILESYSTEM_FILE_H
#define ECO_FILESYSTEM_FILE_H
/*******************************************************************************
@ name
file.

@ function

@ exception

@ note


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2016-05-09.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2015 - 2017, ujoy, reserved all right.

*******************************************************************************/
#include <stdio.h>
#include <sys/stat.h>
#include <eco/Error.h>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(filesystem);
////////////////////////////////////////////////////////////////////////////////
class FileRaw : public eco::Object<FileRaw>
{
public:
	inline FileRaw() : m_fp(nullptr)
	{}

	inline ~FileRaw()
	{
		close();
	}

	inline bool open(
		IN const char* file_path,
		IN const char* open_mode,
		IN eco::String& e)
	{
		close();
		m_fp = ::fopen(file_path, open_mode);
		if (null())
		{
			e << "open file fail: " << file_path;
			return false;
		}
		return true;
	}

	inline void open(
		IN const char* file_path,
		IN const char* open_mode)
	{
		close();
		m_fp = ::fopen(file_path, open_mode);
		if (null())
		{
			ECO_THROW("open file fail: ") << file_path;
		}
	}

	inline void close()
	{
		if (m_fp != nullptr)
		{
			::fclose(m_fp);
			m_fp = nullptr;
		}
	}

	inline bool null() const
	{
		return m_fp == nullptr;
	}

	inline void read(IN char* buf, IN size_t size)
	{
		::fread(buf, 1, size, m_fp);
	}

	inline void write(IN const char* buf, IN size_t size)
	{
		::fwrite(buf, size, 1, m_fp);
	}

	inline void flush()
	{
		::fflush(m_fp);
	}

	inline FILE* get()
	{
		return m_fp;
	}
	inline const FILE* get() const
	{
		return m_fp;
	}

public:
	// compute file size which has been seen as binary.
	inline static uint32_t file_size(IN const char* file_path)
	{
		struct stat buf;
		int result = ::stat(file_path, &buf);
		if (result != 0)
		{
			ECO_THROW("get file size by ::stat() fail.");
		}
		return buf.st_size;
	}

private:
	FILE* m_fp;
};


////////////////////////////////////////////////////////////////////////////////
class File : public FileRaw
{
	ECO_OBJECT(File);
public:
	inline File() : m_file_size(0)
	{}

	inline File(
		IN const char* file_path,
		IN const char* open_mode)
		: m_file_size(0)
	{
		open(file_path, open_mode);
	}

	inline ~File()
	{
		close();
	}

	inline void open(
		IN const char* file_path,
		IN const char* open_mode)
	{
		FileRaw::open(file_path, open_mode);
		m_file_size = FileRaw::file_size(file_path);
	}

	inline void close()
	{
		FileRaw::close();
		m_file_size = 0;
	}

	inline const uint64_t file_size() const
	{
		return m_file_size;
	}

	inline void write(IN const char* buf, IN size_t size)
	{
		FileRaw::write(buf, size);
		m_file_size += size;
	}

	inline void flush()
	{
		FileRaw::flush();
	}

private:
	uint64_t m_file_size;
};


////////////////////////////////////////////////////////////////////////////////
// shim class for read file
class ReadFile
{
	ECO_NONCOPYABLE(ReadFile);
public:
	inline ReadFile()
	{}

	inline ReadFile(
		IN const std::string& file,
		IN const char* mode = "r")
	{
		open(file, mode);
	}

	inline void open(
		IN const std::string& file,
		IN const char* mode = "r")
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
ECO_NS_END(filesystem);
ECO_NS_END(eco);
#endif