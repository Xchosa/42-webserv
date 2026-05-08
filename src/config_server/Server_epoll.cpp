

#include "webserv.hpp"

void Server::addFdEpoll(int fd, uint32_t events)
{
	epoll_event _epollEv{};

	_epollEv.events = events; // epollin, epollout setting mask
	_epollEv.data.fd = fd; 

	if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, fd, &_epollEv) == -1)
		throw std::runtime_error("epoll_ctl ADD failed");
}

void Server::modifyFdEpoll(int fd, uint32_t events)
{
	epoll_event _epollEv{};

	_epollEv.events = events;
	_epollEv.data.fd = fd;

	if (epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, fd, &_epollEv) == -1)
		throw std::runtime_error("epoll_ctl MOD failed");

}
void Server::removeFdEpoll(int fd)
{
    if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, NULL) == -1)
		throw std::runtime_error("epoll_ctl DEL failed (fd invalid,notRegistered,closed)");
}