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
#include <eco/rx/RxExport.h>
#include <list>


ECO_NS_BEGIN(eco);
namespace test{;
////////////////////////////////////////////////////////////////////////////////
class Result
{
public:
	enum State
	{
		success,		// �ɹ�
		failed,			// ʧ��
		timeout,		// ��ʱ
		unknown,		// δ֪����
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
	uint32_t m_state;		// ִ�н��״̬
	std::string m_msg;		// ִ�н����ϸ��Ϣ
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
	// �ܳ�������
	uint32_t m_total_scene;
	// ִ�м���������������
	uint32_t m_checked_scene;
	// ִ��ʧ�ܵ�������
	uint32_t m_failed_scene;

	// ����������
	uint32_t m_total_case;
	// ִ�м���������������
	uint32_t m_checked_case;
	// ִ��ʧ�ܵ�������
	uint32_t m_failed_case;

	// ����������
	uint32_t m_total_test;
	// ִ�м����Ĳ���������
	uint32_t m_checked_test;
	// ִ��ʧ�ܵĲ��ԡ�
	uint32_t m_failed_test;
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif