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




namespace eco{;
namespace test{;


////////////////////////////////////////////////////////////////////////////////
class Suite : public eco::test::Scene
{
	ECO_SINGLETON(Suite);
protected:
	// event: begin
	virtual void OnBegin()
	{
		Runner::OnBegin();

		eco::FixStream fmt;
		fmt << "[##########] ";
		fmt << "suite: ";
		fmt << GetResultSet().m_total_case << " case, ";
		fmt << GetResultSet().m_total_scene << " scene.";
		EcoTrace << fmt.c_str();
	}

	// event: end
	virtual void OnEnd()
	{
		Scene::OnSceneEnd();

		eco::FixStream fmt;
		fmt << "[##########] ";
		fmt << "suite: ";
		fmt << "scene ";
		fmt << GetResultSet().GetTotalChildScene() << "-";
		fmt << GetResultSet().GetCheckedChildScene() << "-";
		fmt << GetResultSet().GetFailedChildScene() << "-";
		fmt << GetResultSet().GetSuccessChildScene() << ", ";
		fmt << "case ";
		fmt << GetResultSet().GetTotalCase() << "-";
		fmt << GetResultSet().GetCheckedCase() << "-";
		fmt << GetResultSet().GetFailedCase() << "-";
		fmt << GetResultSet().GetSuccessCase() << ", ";
		fmt << "test ";
		fmt << GetResultSet().GetTotalTest() << "-";
		fmt << GetResultSet().GetCheckedTest() << "-";
		fmt << GetResultSet().GetFailedTest() << "-";
		fmt << GetResultSet().GetSuccessTest() << ".";
		EcoTrace << fmt.c_str();
	}
};
ECO_SINGLETON_NAME(Suite, GetSuite);


////////////////////////////////////////////////////////////////////////////////
}// ns::test
}// ns::eco
#endif