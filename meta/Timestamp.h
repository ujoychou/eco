#ifndef ECO_META_TIME_STAMP_H
#define ECO_META_TIME_STAMP_H
////////////////////////////////////////////////////////////////////////////////
#include <eco/Export.h>


namespace eco{ ;
namespace meta{ ;


////////////////////////////////////////////////////////////////////////////////
// timestamp state.
enum
{
	v_insert			= 0,		// inserted object, a new object.
	v_update			= 1,		// updated object.
	v_remove			= 2,		// removed object.
	v_clear				= 3,		// original object, a not changed object.
};
typedef uint32_t Timestamp;


////////////////////////////////////////////////////////////////////////////////
inline bool cleared(IN const Timestamp v)
{
	return (v == v_clear);
}
inline bool removed(IN const Timestamp v)
{
	return (v == v_remove);
}
inline bool updated(IN const Timestamp v)
{
	return (v == v_update);
}
inline bool inserted(IN const Timestamp v)
{
	return (v == v_insert);
}


////////////////////////////////////////////////////////////////////////////////
inline void clear(OUT Timestamp& v)
{
	if (!removed(v))
		v = v_clear;
}
inline void remove(OUT Timestamp& v)
{
	if (cleared(v) || updated(v))
		v = v_remove;
}
inline void insert(OUT Timestamp& v)
{
	v = v_insert;
}
inline void update(OUT Timestamp& v)
{
	if (cleared(v))
		v = v_update;
}


////////////////////////////////////////////////////////////////////////////////
// get timestamp text.
inline const char* get_text(IN const Timestamp v)
{
	switch (v)
	{
	case v_clear:
		return "clear";
	case v_insert:
		return "insert";
	case v_update:
		return "update";
	case v_remove:
		return "remove";
	}
	return "none";
}


inline eco::meta::Timestamp& timestamp()
{
	static eco::meta::Timestamp ts = v_clear;
	return ts;
}


////////////////////////////////////////////////////////////////////////////////
}}
#endif