#ifndef XXX_APP_H
#define XXX_APP_H
/*******************************************************************************
@ ����

@ ����

@ �쳣

@ ��ע

--------------------------------------------------------------------------------
@ ��ʷ��¼ @
@ ujoy modifyed on 2016-10-29

--------------------------------------------------------------------------------
* ��Ȩ����(c) 2015 - 2017, ujoychou, ��������Ȩ����

*******************************************************************************/
#include <eco/App.h>
#include <eco/proto/Test.pb.h>

namespace xxx{ ;


////////////////////////////////////////////////////////////////////////////////
class App : public eco::App
{
public:
	// ���캯�����ɳ�ʼ��"sys"��־�ļ���
	App();

protected:
	// ��ʼ��APP
	virtual void on_init() override;
	virtual void on_load() override;

	// �˳�APP
	virtual void on_exit() override;

	// Ĭ��
	void on_default(IN eco::net::Context& c);
	void on_rsp(IN proto::TestRsp& rsp, IN eco::net::Context& c);
	void on_run();

private:
	eco::net::TcpClient m_client;
	eco::Thread m_thread;
	int m_req_id;
};


ECO_APP(App, get_app);
////////////////////////////////////////////////////////////////////////////////
}
#endif