#ifndef ECO_TEST_RESULT_H
#define ECO_TEST_RESULT_H
/*******************************************************************************
@ name

@ function


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2015-06-18.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2013 - 2015, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Export.h>
#include <list>


namespace eco{;
namespace test{;
////////////////////////////////////////////////////////////////////////////////
class Result
{
public:
	enum State
	{
		success,		// 成功
		failed,			// 失败
		timeout,		// 超时
		unknown,		// 未知错误
	};

	Result();

	inline void set_state(State v)
	{
		m_state = v;
	}

	inline uint32_t state() const
	{
		return m_state;
	}

private:
	uint32_t m_state;		// 执行结果状态
	std::string m_msg;		// 执行结果详细信息
};


////////////////////////////////////////////////////////////////////////////////
class ResultSet
{
////////////////////////////////////////////////////////////////////////////////
public:
	ResultSet()
	{
		reset();
	}

	inline void reset()
	{
		reset_case();
		reset_scene();
		ResetTest();
	}

	inline void ResetTest()
	{
		m_total_test = 0;
		m_failed_test = 0;
		m_checked_test = 0;
	}

	inline void reset_case()
	{
		m_total_case = 0;
		m_failed_case = 0;
		m_checked_case = 0;
	}

	inline void reset_scene()
	{
		m_total_scene = 0;
		m_failed_scene = 0;
		m_checked_scene = 0;
	}

////////////////////////////////////////////////////////////////////////////////
public:
	inline void add_fail(uint32_t v = 1)
	{
		m_checked_test += v;
		m_failed_test += v;
		m_total_test += v;
	}

	inline void add_success(uint32_t v = 1)
	{
		m_checked_test += v;
		m_total_test += v;
	}
	
	inline bool ok() const
	{
		return m_failed_test == 0;
	}

	inline uint32_t total_test() const
	{
		return m_total_test;
	}

	inline uint32_t checked_test() const
	{
		return m_checked_test;
	}

	inline uint32_t failed_test() const
	{
		return m_failed_test;
	}

	inline uint32_t success_test() const
	{
		return m_total_test - m_failed_test;
	}

////////////////////////////////////////////////////////////////////////////////
public:
	inline void set_total_case(uint32_t v)
	{
		m_total_case = v;
	}

	inline void set_checked_case(uint32_t v)
	{
		m_checked_case = v;
	}

	inline void set_failed_case(uint32_t v)
	{
		m_failed_case = v;
	}

	inline uint32_t total_case() const
	{
		return m_total_case;
	}

	inline uint32_t checked_case() const
	{
		return m_checked_case;
	}

	inline uint32_t failed_case() const
	{
		return m_failed_case;
	}

	inline uint32_t success_case() const
	{
		return m_total_case - m_failed_case;
	}

////////////////////////////////////////////////////////////////////////////////
public:
	inline void set_total_scene(uint32_t v)
	{
		m_total_scene = v;
	}

	inline void set_checked_scene(uint32_t v)
	{
		m_checked_scene = v;
	}

	inline void set_failed_scene(uint32_t v)
	{
		m_failed_scene = v;
	}

	inline uint32_t checked_scene() const
	{
		return m_checked_scene;
	}

	inline uint32_t failed_child_scene() const
	{
		return (m_failed_scene == 0) ? 0 : m_failed_scene - 1;
	}

	inline uint32_t total_child_scene() const
	{
		return (m_total_scene == 0) ? 0 : m_total_scene - 1;
	}

	inline uint32_t checked_child_scene() const
	{
		return (m_checked_scene == 0) ? 0 : m_checked_scene - 1;
	}

	inline uint32_t success_child_scene() const
	{
		return total_child_scene() - failed_child_scene();
	}
	
////////////////////////////////////////////////////////////////////////////////
public:
	// 总场景数。
	uint32_t m_total_scene;
	// 执行检测过的用例数量。
	uint32_t m_checked_scene;
	// 执行失败的用例。
	uint32_t m_failed_scene;

	// 用例总数。
	uint32_t m_total_case;
	// 执行检测过的用例数量。
	uint32_t m_checked_case;
	// 执行失败的用例。
	uint32_t m_failed_case;

	// 测试总数。
	uint32_t m_total_test;
	// 执行检测过的测试数量。
	uint32_t m_checked_test;
	// 执行失败的测试。
	uint32_t m_failed_test;
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif