#ifndef ECO_META_STAMP_H
#define ECO_META_STAMP_H
////////////////////////////////////////////////////////////////////////////////
#include <eco/rx/RxExport.h>


namespace eco{ ;
namespace meta{ ;
////////////////////////////////////////////////////////////////////////////////
// stamp state.
enum
{
	stamp_insert	= 0,		// insert object.(new object)
	stamp_update	= 1,		// update object.
	stamp_delete	= 2,		// delete object.
	stamp_clean		= 3,		// clean  object.(not update)

	stamp_silent	= 8,		// publish content silently.
};
typedef uint8_t Stamp;


////////////////////////////////////////////////////////////////////////////////
inline bool is_clean(IN const Stamp v)
{
	return (v == stamp_clean);
}
inline bool is_remove(IN const Stamp v)
{
	return (v == stamp_delete);
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
	if (is_clean(v) || is_update(v)) v = stamp_delete;
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
	case stamp_delete:
		return "remove";
	}
	return "none";
}


////////////////////////////////////////////////////////////////////////////////
}}
#endif