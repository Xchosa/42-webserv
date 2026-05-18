#include "Server.hpp"

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



bool Server::isServerFd(int fd) const
{
	return (this->_socket_fds.find(fd) != _socket_fds.end());
}

void Server::closeClient(int client_fd)
{
	std::cout << "close client fd: " << client_fd << std::endl;
	removeFdEpoll(client_fd);
	close(client_fd);
	_clients.erase(client_fd);
}


void Server::checkClientTimeouts()
{
	std::vector<int> timed_out_clients;

	for (const auto& [client_fd, client] : _clients)
	{
		if (checklastActivity(client_fd) >= TIMEOUT)
			timed_out_clients.emplace_back(client_fd);
	}

	for (int client_fd : timed_out_clients)
	{
		closeClient(client_fd);
	}
  }
