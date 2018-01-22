#ifndef ECO_FILE_H
#define ECO_FILE_H
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
#include <eco/Project.h>
#include <stdio.h>
#include <sys/stat.h>



namespace eco{;
namespace filesystem{;


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
		IN eco::Error& e)
	{
		close();
		m_fp = ::fopen(file_path, open_mode);
		if (null())
		{
			e.id(-1) << "open file fail: " << file_path;
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
			EcoThrow << "open file fail: " << file_path;
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
			EcoThrow << "get file size by ::stat() fail.";
		}
		return buf.st_size;
	}

private:
	FILE* m_fp;
};


////////////////////////////////////////////////////////////////////////////////
class File : public FileRaw
{
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


}}
#endif