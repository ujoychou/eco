#ifndef ECO_FILESYSTEM_PATH_H
#define ECO_FILESYSTEM_PATH_H
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
#include <cstring>
#include <eco/Type.h>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(filesystem);
////////////////////////////////////////////////////////////////////////////////
inline const char* filename(const char* fullname)
{
	const char* name = nullptr;
	return nullptr != (name = strrchr(fullname, '/'))
		|| nullptr != (name = strrchr(fullname, '\\'))
		? (name + 1) : fullname;
}


////////////////////////////////////////////////////////////////////////////////
inline void format(OUT std::string& p)
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
		// ignore repeated '\\'
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
inline bool equal(IN const std::string& p1, IN const std::string& p2)
{
	std::string cmp1(p1);
	std::string cmp2(p2);
	format(cmp1);
	format(cmp2);
	return eco::iequal(cmp1.c_str(), cmp2.c_str());
}


////////////////////////////////////////////////////////////////////////////////
inline void add_path_suffix(std::string& path_)
{
	if (!path_.empty() || (path_.back() != '/' && path_.back() != '\\'))
	{
		path_ += '\\';
	}
}


////////////////////////////////////////////////////////////////////////////////
inline std::string parent_path(const std::string& path)
{
	// if format like "c:\good\bad\".
	size_t off = (path.back() == '/' || path.back() == '\\')
		? (path.size() - 2) : std::string::npos;
	size_t pos = path.find_last_of('\\', off);
	size_t pos_x = path.find_last_of('/', off);
	if (pos_x > pos && pos_x != std::string::npos) pos = pos_x;
	return (pos != std::string::npos) ? path.substr(0, pos + 1) : path;
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(filesystem);
ECO_NS_END(eco);
#endif