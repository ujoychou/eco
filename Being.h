#ifndef ECO_BEING_H
#define ECO_BEING_H
/*******************************************************************************
@ name


@ function


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2013-01-01.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2013 - 2015, ujoy, reserved all right.

*******************************************************************************/
#include <eco/ExportApi.h>
#include <eco/Object.h>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
class ECO_API Being
{
	ECO_IMPL_API();
	ECO_OBJECT(Being);
public:
	// ��������λ�����Ƶ��
	static const uint32_t get_unit_live_tick_seconds();

	// ����������Ĭ��ʱ�� = 10 seconds.
	Being(IN uint32_t live_secs = 10, IN const char* name = "");

	// ��������
	virtual ~Being();

	// ��ʼ����
	void born();

	// ��������
	void kill();

	// ��ʼ�����
	void live();

	// �����Ƿ��ѿ�ʼ
	bool is_born() const;

	// �����Ƶ�ʣ���Tick��Seconds��
	void set_live_ticks(IN uint32_t ticks);
	uint32_t get_live_ticks() const;
	void set_live_seconds(IN uint32_t secs);
	uint32_t get_live_seconds() const;

	// �����Ƶ�ʣ�ʵ������ʱ��
	uint32_t get_living_seconds() const;

	// ����������
	void set_name(IN const char*);
	const char* get_name() const;
	Being& name(IN const char*);

protected:
	// ������������ʼ��
	virtual bool on_born()
	{
		return true;
	}

	// άϵ��������޸�
	virtual void on_live()
	{}

	// �Ƿ����л����
	virtual bool to_live()
	{
		return true;
	}

	friend class Eco;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif