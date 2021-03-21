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
#include <eco/test/Runner.h>
#include <eco/test/Case.h>
#include <list>




namespace eco{;
namespace test{;
////////////////////////////////////////////////////////////////////////////////
class Scene : public Runner
{
	ECO_OBJECT(Scene);
public:
	typedef std::list<std::shared_ptr<Runner> >::iterator iterator;

	// constructor.
	inline Scene()
	{}

	// constructor.
	inline Scene(IN const std::string& name) : Runner(name)
	{}

	virtual ~Scene()
	{}

	virtual uint32_t count_scene() const override
	{
		uint32_t count = 1;		// current scene.
		if (!has_child()) return count;
		for (auto it = children()->begin(); it!= children()->end(); ++it)
		{
			count += it->get()->count_scene();
		}
		return count;
	}

	virtual const RunnerChildren* children() const override
	{
		return &m_runner_list;
	}

	// add test case.
	template<typename case_type>
	inline case_type* add_case(
		IN const std::string& case_name)
	{
		case_type* new_case = NULL;
		std::shared_ptr<Runner> case_obj(new_case = new case_type());
		new_case->set_name(case_name);
		new_case->set_owner(*this);
		m_runner_list.push_back(case_obj);
		return new_case;
	}

	// add test case.
	inline Case* add_case(
		IN const std::string& case_name,
		IN RunnerFunc case_func,
		IN std::shared_ptr<Object> obj)
	{
		Case* new_case = NULL;
		std::shared_ptr<Runner> case_obj(
			new_case = new CaseImpl(case_name, case_func, obj));
		new_case->set_owner(*this);
		m_runner_list.push_back(case_obj);
		return new_case;
	}

	// find exist scene in runner tree.
	inline Case* find_case(IN const std::string& case_name)
	{
		for (auto it = m_runner_list.begin(); it!=m_runner_list.end(); ++it)
		{
			std::shared_ptr<Scene> scene =
				std::dynamic_pointer_cast<Scene>(*it);
			if (scene != nullptr)
			{
				Case* case_obj = scene->find_case(case_name);
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
	inline Scene::ptr get_child(IN const std::string& scene_name)
	{
		std::shared_ptr<Scene> scene = find_child(scene_name);
		if (scene == nullptr)
		{
			scene.reset(new Scene(scene_name));
			add_scene(scene);
		}
		return scene;
	}
	// get or create new a scene.
	inline void add_scene(IN std::shared_ptr<Scene>& scene)
	{
		scene->set_owner(*this);
		m_runner_list.push_back(scene);
	}

	// get or create new a scene.
	inline Scene::ptr find_child(IN const std::string& scene_name)
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
	inline Scene::ptr find_scene(IN const std::string& scene_name)
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
					scene = scene->find_scene(scene_name);
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
	inline void release()
	{
		m_runner_list.clear();
	}

	inline void on_scene_end()
	{
		Runner::on_end();

		// update scene info.
		if (!result_set().ok())
		{
			result_set().m_failed_scene += 1;
		}
		result_set().m_checked_scene += 1;
	}

////////////////////////////////////////////////////////////////////////////////
protected:
	virtual void on_begin() override
	{
		Runner::on_begin();

		eco::String fmt;
		fmt << "[==========] " << fullname() << ": ";
		fmt << result_set().m_total_case << " case, ";
		fmt << result_set().m_total_scene << " scene.";
		ECO_INFO << fmt.c_str();
	}

	virtual void on_end() override
	{
		on_scene_end();
		
		// show count info.
		eco::String fmt;
		fmt << "[==========] " << fullname() << ": ";
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

private:
	RunnerChildren m_runner_list;
};


////////////////////////////////////////////////////////////////////////////////
// add case to scene.
#define ECO_ADD_CASE(scene, case_name, bind_func, obj) \
scene->add_case(case_name, std::bind(bind_func, obj), \
	std::dynamic_pointer_cast<eco::test::Object>(obj))

////////////////////////////////////////////////////////////////////////////////
}}
#endif