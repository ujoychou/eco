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
	original	= 0,		// original object, a not changed object.
	inserted	= 1,		// inserted object, a new object.
	updated		= 2,		// updated object.
	removed		= 3,		// removed object.
};
typedef uint16_t TimestampState;


////////////////////////////////////////////////////////////////////////////////
class Timestamp
{
public:
	/*@ constructor. */
	explicit Timestamp(IN const uint16_t v = inserted) : m_timestamp(v)
	{}

	// reset object a original state.
	inline void origin()
	{
		if (!is_removed())
			m_timestamp = original;
	}
	inline bool is_original() const
	{
		return (m_timestamp == original);
	}

	// set object a inserted state which is a new object.
	inline void insert()
	{
		m_timestamp = inserted;
	}
	inline bool is_inserted() const
	{
		return (m_timestamp == inserted);
	}

	// updated state.
	inline void update()
	{
		if (is_original())
			m_timestamp = updated;
	}
	inline bool is_updated() const
	{
		return (m_timestamp == updated);
	}

	// removed state. 
	inline void remove()
	{
		if (is_original() || is_updated())
		{
			m_timestamp = removed;
		}
	}
	inline bool is_removed() const
	{
		return (m_timestamp == removed);
	}

	inline Timestamp& timestamp()
	{
		return *this;
	}

	// get timestamp text.
	inline const char* get_text() const
	{
		switch (m_timestamp)
		{
		case original:
			return "ori";
		case inserted:
			return "ins";
		case updated:
			return "upd";
		case removed:
			return "rem";
		}
		return "non";
	}

	// set timestamp value.
	inline void set_value(IN const uint16_t v)
	{
		m_timestamp = v;
	}
	inline uint16_t get_value() const
	{
		return m_timestamp;
	}

private:
	uint16_t m_timestamp;
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif