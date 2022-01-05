#ifndef ECO_TEST_CHECK_H
#define ECO_TEST_CHECK_H
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
#include <eco/test/Trace.h>


ECO_NS_BEGIN(eco);
namespace test{;
////////////////////////////////////////////////////////////////////////////////
class TestCheck
{
public:
	virtual eco::String format() const = 0;
};


////////////////////////////////////////////////////////////////////////////////
class TestLogicFailed : public TestCheck
{
public:
	std::string m_detail;
	eco::test::Trace m_tracer;

public:
	inline void set_detail(IN const std::string& expr_detail)
	{
		m_detail = expr_detail;
	}

	virtual eco::String format() const override
	{
		eco::String fmt;
		fmt << "[   fail   ] ";
		fmt << "false = expect : actual ";
		fmt << "{" << m_detail << "}";
		fmt << "{" << m_tracer.format() << "}";
		return std::move(fmt);
	}
};


////////////////////////////////////////////////////////////////////////////////
class TestEqualFailed : public TestLogicFailed
{
public:
	std::string m_expect;
	std::string m_actual;

public:
	inline void set_detail(
		IN const std::string& expect_detail,
		IN const std::string& actual_detail)
	{
		eco::Buffer<> buf;
		buf << expect_detail << "!=" << actual_detail;
		m_detail = buf;
	}

	virtual eco::String format() const override
	{
		eco::String buf;
		buf << "[   fail   ] " << m_expect
		<< " = expect : actual = " << m_actual << " "
		<< "{" << m_detail << "}"
		<< "{" << m_tracer.format() << "}";
		return std::move(buf);
	}
};



////////////////////////////////////////////////////////////////////////////////
class TestLogicSuccess : public TestCheck
{
public:
	virtual eco::String format() const override
	{
		eco::String fmt;
		fmt << "[   pass   ] " << "true  = expect : actual";
		return std::move(fmt);
	}
};

////////////////////////////////////////////////////////////////////////////////
class TestEqualSuccess : public TestLogicSuccess
{
public:
	std::string m_expect;

public:
	virtual eco::String format() const override
	{
		eco::String fmt;
		fmt << "[   pass   ] " << m_expect << " = expect : actual";
		return std::move(fmt);
	}
};


////////////////////////////////////////////////////////////////////////////////
#define ECO_EXPECT_LOGIC_GET_RESULT(test_res, logic_expr) \
{\
	eco::test::Trace tracer(__FILE__, __LINE__, __FUNCTION__);\
	if (logic_expr) \
	{\
		GetResultSet().Success();\
		eco::test::TestLogicSuccess* succ_res = nullptr;\
		test_res.reset(succ_res = new eco::test::TestLogicSuccess);\
	}\
	else\
	{\
		GetResultSet().Fail();\
		eco::test::TestLogicFailed* fail_res = nullptr;\
		test_res.reset(fail_res = new eco::test::TestLogicFailed);\
		fail_res->m_tracer = tracer;\
		fail_res->SetDetail(#logic_expr);\
	}\
}


////////////////////////////////////////////////////////////////////////////////
#define ECO_EXPECT_EQUAL_GET_RESULT(test_res, expect, actual) \
{\
	eco::test::Trace tracer(__FILE__, __LINE__, __FUNCTION__);\
	auto actual_val = (actual); \
	if (expect == actual_val) \
	{\
		GetResultSet().Success();\
		eco::test::TestEqualSuccess* succ_res = nullptr;\
		test_res.reset(succ_res = new eco::test::TestEqualSuccess);\
		std::stringstream fmt;\
		fmt << expect;\
		succ_res->m_expect = fmt.str();\
	}\
	else\
	{\
		GetResultSet().Fail();\
		eco::test::TestEqualFailed* fail_res = nullptr;\
		test_res.reset(fail_res = new eco::test::TestEqualFailed);\
		std::stringstream fmt;\
		fmt << expect;\
		fail_res->m_expect = fmt.str();\
		fmt.str("");\
		fmt << actual_val;\
		fail_res->m_actual = fmt.str();\
		fail_res->m_tracer = tracer;\
		fail_res->SetDetail(#expect, #actual);\
	}\
}


////////////////////////////////////////////////////////////////////////////////
#define ECO_EXPECT_LOGIC(logic_expr) \
{\
	std::shared_ptr<eco::test::TestCheck> test_res; \
	ECO_EXPECT_LOGIC_GET_RESULT(test_res, logic_expr);\
	ECO_INFO << test_res->Format();\
}


////////////////////////////////////////////////////////////////////////////////
#define ECO_EXPECT_EQUAL(expect, actual) \
{\
	std::shared_ptr<eco::test::TestCheck> test_res; \
	ECO_EXPECT_EQUAL_GET_RESULT(test_res, expect, actual);\
	ECO_INFO << test_res->Format();\
}


////////////////////////////////////////////////////////////////////////////////
}}
#endif