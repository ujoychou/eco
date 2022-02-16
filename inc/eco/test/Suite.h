#ifndef ECO_TEST_SUITE_H
#define ECO_TEST_SUITE_H
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
#include <eco/test/Case.h>
#include <eco/test/Scene.h>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(test);
////////////////////////////////////////////////////////////////////////////////
class Suite : public eco::test::Scene
{
	ECO_SINGLETON(Suite);
protected:
	// event: begin
	virtual void on_begin() override
	{
		Runner::on_begin();

		eco::String fmt;
		fmt << "[##########] ";
		fmt << "suite: ";
		fmt << result_set().m_total_case << " case, ";
		fmt << result_set().m_total_scene << " scene.";
		ECO_INFO << fmt.c_str();
	}

	// event: end
	virtual void on_end() override
	{
		Scene::on_scene_end();

		eco::String fmt;
		fmt << "[##########] ";
		fmt << "suite: ";
		fmt << "scene ";
		fmt << result_set().total_child_scene() << "-";
		fmt << result_set().checked_child_scene() << "-";
		fmt << result_set().failed_child_scene() << "-";
		fmt << result_set().success_child_scene() << ", ";
		fmt << "case ";
		fmt << result_set().total_case() << "-";
		fmt << result_set().checked_case() << "-";
		fmt << result_set().failed_case() << "-";
		fmt << result_set().success_case() << ", ";
		fmt << "test ";
		fmt << result_set().total_test() << "-";
		fmt << result_set().checked_test() << "-";
		fmt << result_set().failed_test() << "-";
		fmt << result_set().success_test() << ".";
		ECO_INFO << fmt.c_str();
	}
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(test);
ECO_NS_END(eco);
#endif