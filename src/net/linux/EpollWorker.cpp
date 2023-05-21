#include "Pch.h"
#include "EpollWorker.h"
////////////////////////////////////////////////////////////////////////////////


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(net);
////////////////////////////////////////////////////////////////////////////////
void EpollWorker::work()
{
	this->running = true;
	while (this->running)
	{
		this->active_events.clear();
		int size = ::epoll_wait(this->fd_epoll, &this->active_events[0],
								(int)this->active_events.size(), -1);
		for (int i = 0; i < size; ++i)
		{
			struct epoll_event& event = this->active_events[i];
			// event triger: notify
			if (event.data.fd == this->fd_notify)
			{
				this->notify_read();
			}
			// edge triger: accept
			else if (event.data.fd == this->accept->listen.fd)
			{
				LinuxSocket::ref socket = this->accept->accept();
				if (socket == nullptr)
				{
					this->event->on_error();
				}
				this->event->on_accept(socket);
			}
			// edge triger: read
			else if (event.events & (EPOLLIN | EPOLLPRI |EPOLLRDHUP))
			{
				LinuxSocket::ref socket = get_socket(event);
				bool error = set_error(socket, event.events);
				this->event->on_read(socket, error);
			}
			// edge triger: write
			else if (event.events & (EPOLLOUT))
			{
				LinuxSocket::ref socket = get_socket(event);
				bool error = set_error(socket, event.events);
				this->event->on_write(socket, error);
			}
			// edge triger: close
			else if (event.events & (EPOLLHUP) && !(event.events & EPOLLIN))
			{
				LinuxSocket::ref socket = get_socket(event);
				bool error = set_error(socket, event.events);
				this->event->on_close(socket, error);
			}
		}// end for()

		// run tasks
		std::vector<eco::Task> runs;
		{
			std::lock_guard<std::mutex> lock(this->mutex);
			runs = this->tasks;
		}
		for (eco::Task& task : runs) { task(); }
	}// end while()
}


////////////////////////////////////////////////////////////////////////////////
void EpollWorker::notify_poll()
{
	uint64_t value = 1;
	size_t size = ::write(this->fd_notify, &value, sizeof(value));
	if (size != sizeof(value))
	{
		throw(-1);
	}
}
void EpollWorker::notify_read()
{
	uint64_t value = 1;
	size_t size = ::read(this->fd_notify, &value, sizeof(value));
	if (size != sizeof(value))
	{
		throw(-1);
	}
}


////////////////////////////////////////////////////////////////////////////////
void EpollWorker::bind_accept(LinuxAccept& accept)
{
	this->accept = &accept;
	this->socket_listen_add(this->accept->listen.fd);
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(net);
ECO_NS_END(eco);
