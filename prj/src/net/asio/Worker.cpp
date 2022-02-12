#include "Pch.h"
#include <eco/net/Worker.h>
#include <eco/net/asio/Worker.h>
////////////////////////////////////////////////////////////////////////////////



ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(net);
////////////////////////////////////////////////////////////////////////////////
class Worker::Impl
{
	ECO_IMPL_INIT(Worker);
public:
	asio::Worker m_worker;
};

////////////////////////////////////////////////////////////////////////////////
ECO_OBJECT_IMPL(Worker);
void Worker::run(const char* name)
{
	m_impl->m_worker.run(name);
}

void Worker::join()
{
	m_impl->m_worker.join();
}

void Worker::stop()
{
	m_impl->m_worker.stop();
}

bool Worker::stopped() const
{
	return m_impl->m_worker.stopped();
}
bool Worker::running() const
{
	return m_impl->m_worker.running();
}
bool Worker::initing() const
{
	return m_impl->m_worker.initing();
}

void Worker::async_stop()
{
	m_impl->m_worker.async_stop();
}

IoWorker* Worker::get_io_worker()
{
	return (IoWorker*)&m_impl->m_worker;
}

IoService* Worker::get_io_service()
{
	return (IoService*)m_impl->m_worker.get_io_service();
}


////////////////////////////////////////////////////////////////////////////////
}}