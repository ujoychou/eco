#ifndef ECO_TEST_SCENE_H
#define ECO_TEST_SCENE_H
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
#include <eco/Project.h>
#include <eco/test/Runner.h>
#include <eco/test/Case.h>
#include <list>




namespace eco{;
namespace test{;

////////////////////////////////////////////////////////////////////////////////
class Scene : public Runner
{
////////////////////////////////////////////////////////////////////////////////
public:
	typedef std::list<std::shared_ptr<Runner> >::iterator iterator;

	// constructor.
	inline Scene()
	{}

	// constructor.
	inline Scene(IN const std::string& name) 
		: Runner(name)
	{}

	virtual ~Scene()
	{}

	virtual uint32_t CountScene() const
	{
		uint32_t count = 1;		// current scene.
		RunnerChildren::const_iterator it = GetChildren().begin();
		for (; it!=GetChildren().end(); ++it)
		{
			count += it->get()->CountScene();
		}
		return count;
	}

	virtual const RunnerChildren& GetChildren() const override
	{
		return m_runner_list;
	}

	// add test case.
	template<typename case_type>
	inline case_type* AddCase(
		IN const std::string& case_name)
	{
		case_type* new_case = NULL;
		std::shared_ptr<Runner> case_obj(new_case = new case_type());
		new_case->SetName(case_name);
		new_case->SetOwner(*this);
		m_runner_list.push_back(case_obj);
		return new_case;
	}

	// add test case.
	inline Case* AddCase(
		IN const std::string& case_name,
		IN RunnerFunc case_func,
		IN std::shared_ptr<Object> obj)
	{
		Case* new_case = NULL;
		std::shared_ptr<Runner> case_obj(
			new_case = new CaseImpl(case_name, case_func, obj));
		new_case->SetOwner(*this);
		m_runner_list.push_back(case_obj);
		return new_case;
	}

	// find exist scene in runner tree.
	inline Case* FindCase(
		IN const std::string& case_name)
	{
		std::list<std::shared_ptr<Runner>>::iterator it;
		it = m_runner_list.begin();
		for (; it!=m_runner_list.end(); ++it)
		{
			std::shared_ptr<Scene> scene =
				std::dynamic_pointer_cast<Scene>(*it);
			if (scene != nullptr)
			{
				Case* case_obj = scene->FindCase(case_name);
				if (case_obj != nullptr)
				{
					return case_obj;
				}
			}
			else if (**it == case_name)
			{
				return std::dynamic_pointer_cast<Case>(*it).get();
			}
		}
		return nullptr;
	}

	// get or create new a scene.
	inline std::shared_ptr<Scene> GetChild(
		IN const std::string& scene_name)
	{
		std::shared_ptr<Scene> scene = FindChild(scene_name);
		if (scene == nullptr)
		{
			scene.reset(new Scene(scene_name));
			AddScene(scene);
		}
		return scene;
	}
	// get or create new a scene.
	inline void AddScene(
		IN std::shared_ptr<Scene>& scene)
	{
		scene->SetOwner(*this);
		m_runner_list.push_back(scene);
	}

	// get or create new a scene.
	inline std::shared_ptr<Scene> FindChild(
		IN const std::string& scene_name)
	{
		iterator it = m_runner_list.begin();
		for (; it!=m_runner_list.end(); ++it)
		{
			std::shared_ptr<Scene> scene =
				std::dynamic_pointer_cast<Scene>(*it);
			if (scene != nullptr && (**it) == scene_name)
			{
				return scene;
			}
		}
		return nullptr;
	}

	// find exist scene in runner tree.
	inline std::shared_ptr<Scene> FindScene(
		IN const std::string& scene_name)
	{
		std::shared_ptr<Scene> scene;
		iterator it = m_runner_list.begin();
		for (; it!=m_runner_list.end(); ++it)
		{
			scene = std::dynamic_pointer_cast<Scene>(*it);
			if (scene != nullptr)
			{
				if ((**it) == scene_name)
				{
					return scene;
				}
				else
				{
					scene = scene->FindScene(scene_name);
					if (scene != nullptr)
					{
						return scene;
					}
				}
			}// end if (scene != nullptr)
		}
		return std::shared_ptr<Scene>();
	}

	// release runner and it's resource.
	inline void Release()
	{
		m_runner_list.clear();
	}

	inline void OnSceneEnd()
	{
		Runner::OnEnd();

		// update scene info.
		if (!GetResultSet().IsOk())
		{
			GetResultSet().m_failed_scene += 1;
		}
		GetResultSet().m_checked_scene += 1;
	}

////////////////////////////////////////////////////////////////////////////////
protected:
	virtual void OnBegin() override
	{
		Runner::OnBegin();

		eco::FixStream fmt;
		fmt << "[==========] " << GetFullName() << ": ";
		fmt << GetResultSet().m_total_case << " case, ";
		fmt << GetResultSet().m_total_scene << " scene.";
		EcoTrace << fmt.c_str();
	}

	virtual void OnEnd() override
	{
		OnSceneEnd();
		
		// show count info.
		eco::FixStream fmt;
		fmt << "[==========] " << GetFullName() << ": ";
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

private:
	RunnerChildren m_runner_list;
};


////////////////////////////////////////////////////////////////////////////////
// add case to scene.
#define ECO_ADD_CASE(scene, case_name, bind_func, obj) \
scene->AddCase(case_name, std::bind(bind_func, obj), \
	std::dynamic_pointer_cast<eco::test::Object>(obj))

////////////////////////////////////////////////////////////////////////////////
}}
#endif