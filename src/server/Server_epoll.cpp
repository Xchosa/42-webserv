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

bool Server::isCgiPipeFd(int fd) const
{
	return (this->_cgi_fd_client_owner.find(fd) != this->_cgi_fd_client_owner.end());
}

void Server::closeClient(int client_fd)
{
	std::cout << "[INFO]  Client " << client_fd << " closed" << std::endl;
	if (_clients[client_fd]._cgi.has_value())
	{
		killCgi(client_fd, false);
	}
	removeFdEpoll(client_fd);
	close(client_fd);
	_clients.erase(client_fd);
}

void Server::killCgi(int client_fd, bool build_error_response)
{
	ClientInfos*	client = &_clients[client_fd];				// current client

	std::cout << "[INFO]  CGI KILL client " << client_fd << std::endl;

	if (client->_cgi.has_value())
	{
		CgiSession* cgi = &client->_cgi.value(); // cgi session from the client

		// kill cgi child process
		int status;
		kill(cgi->_pid, SIGKILL);
		waitpid(cgi->_pid, &status, 0);
		cgi->_pid = -1;

		// close FDs and remove from owner map
		if (cgi->_stdin_fd != -1)
		{
			removeFdEpoll(cgi->_stdin_fd);
			close(cgi->_stdin_fd);
			_cgi_fd_client_owner.erase(cgi->_stdin_fd);
			cgi->_stdin_fd = -1;
		}
		if (cgi->_stdout_fd != -1)
		{
			removeFdEpoll(cgi->_stdout_fd);
			close(cgi->_stdout_fd);
			_cgi_fd_client_owner.erase(cgi->_stdout_fd);
			cgi->_stdout_fd = -1;
		}
	}
	client->_cgi.reset();

	if (build_error_response == true)
	{
		client->_response = _dispatcher.buildErrorResponse(502, client->_selected_server, CON_KEEP_ALIVE, client->_parser.getRequest());
		modifyFdEpoll(client_fd, EPOLLOUT | EPOLLRDHUP);
	}
}

void Server::checkClientTimeouts()
{
	std::vector<int> timed_out_clients;

	for (const auto& [client_fd, client] : _clients)
	{
		if (checklastActivity(client_fd) >= KEEP_ALIVE_TIMEOUT)
			timed_out_clients.emplace_back(client_fd);
	}

	for (int client_fd : timed_out_clients)
	{
		closeClient(client_fd);
	}
  }
