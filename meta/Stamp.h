#ifndef ECO_META_STAMP_H
#define ECO_META_STAMP_H
////////////////////////////////////////////////////////////////////////////////
#include <eco/Export.h>


namespace eco{ ;
namespace meta{ ;


////////////////////////////////////////////////////////////////////////////////
// stamp state.
enum
{
	stamp_insert	= 0,		// insert object.(new object)
	stamp_update	= 1,		// update object.
	stamp_remove	= 2,		// remove object.
	stamp_clean		= 3,		// clean  object.(not update)
};
typedef uint16_t Stamp;


////////////////////////////////////////////////////////////////////////////////
inline bool is_clean(IN const Stamp v)
{
	return (v == stamp_clean);
}
inline bool is_remove(IN const Stamp v)
{
	return (v == stamp_remove);
}
inline bool is_update(IN const Stamp v)
{
	return (v == stamp_update);
}
inline bool is_insert(IN const Stamp v)
{
	return (v == stamp_insert);
}


////////////////////////////////////////////////////////////////////////////////
inline void clean(OUT Stamp& v)
{
	if (!is_remove(v)) v = stamp_clean;
}
inline void remove(OUT Stamp& v)
{
	if (is_clean(v) || is_update(v)) v = stamp_remove;
}
inline void insert(OUT Stamp& v)
{
	v = stamp_insert;
}
inline void update(OUT Stamp& v)
{
	if (is_clean(v)) v = stamp_update;
}


////////////////////////////////////////////////////////////////////////////////
// get stamp text.
inline const char* get_text(IN const Stamp v)
{
	switch (v)
	{
	case stamp_clean:
		return "clean";
	case stamp_insert:
		return "insert";
	case stamp_update:
		return "update";
	case stamp_remove:
		return "remove";
	}
	return "none";
}


////////////////////////////////////////////////////////////////////////////////
inline eco::meta::Stamp& stamp()
{
	static Stamp v = stamp_clean;
	return v;
}


////////////////////////////////////////////////////////////////////////////////
}}
#endif