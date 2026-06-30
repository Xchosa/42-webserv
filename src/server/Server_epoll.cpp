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
		killCgi(client_fd, 0);
	}
	removeFdEpoll(client_fd);
	close(client_fd);
	_clients.erase(client_fd);
}

void Server::killCgi(int client_fd, int error_code)
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

	if (error_code > 0)
	{
		client->_response = _dispatcher.buildErrorResponse(error_code, client->_selected_server, CON_KEEP_ALIVE, client->_parser.getRequest());
		modifyFdEpoll(client_fd, EPOLLOUT | EPOLLRDHUP);
	}
}

void Server::checkClientTimeouts()
{
	std::vector<int> timed_out_clients;

	for (const auto& [client_fd, client] : _clients)
	{
		if (!client._cgi.has_value() && checklastActivity(client_fd) >= KEEP_ALIVE_TIMEOUT)
			timed_out_clients.emplace_back(client_fd);
	}

	for (int client_fd : timed_out_clients)
	{
		std::cout << "[INFO]  Client " << client_fd << " timed out" << std::endl;
		closeClient(client_fd);
	}
}

void Server::checkCgiTimeouts()
{
	std::vector<int> timed_out_cgis;
	std::vector<int> cgi_finished_but_running;

	for (const auto& [client_fd, client] : _clients)
	{
		if (client._cgi.has_value() && !client._cgi.value()._waited)
		{
			const CgiSession *cgi = &client._cgi.value();
			if (cgi->_stdout_fd != -1) // cgi still running
			{
				if ((time(NULL) - cgi->_started) > CGI_TIMEOUT)
					timed_out_cgis.emplace_back(client_fd);
			}
			else if (cgi->_stdout_fd == -1) // cgi finished but child still running
				cgi_finished_but_running.emplace_back(client_fd);
		}
	}

	for (int client_fd : cgi_finished_but_running)
	{
		ClientInfos* 	client = &_clients[client_fd];
		CgiSession*		cgi = &client->_cgi.value();

		int status;
		pid_t r = waitpid(cgi->_pid, &status, WNOHANG);
		if (r == cgi->_pid) // child finished now
		{
			if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
				client->_response = _dispatcher.buildErrorResponse(502, client->_selected_server, CON_KEEP_ALIVE, client->_parser.getRequest());
			else
				client->_response = _dispatcher.parseCgiOutput(cgi->_output);
			modifyFdEpoll(client_fd, EPOLLOUT | EPOLLRDHUP);
			client->_parser.reset();
			client->_cgi.reset();
			cgi = nullptr;
		}
		else if (r == 0) // child still running, check for timeout
		{
			if ((time(NULL) - cgi->_started) > CGI_TIMEOUT)
			{
				std::cout << "[INFO]  CGI of client " << client_fd << " timed out" << std::endl;
				killCgi(client_fd, 504);
			}
		}
		else // waitpid error
			killCgi(client_fd, 502);
	}

	for (int client_fd : timed_out_cgis)
	{
		std::cout << "[INFO]  CGI of client " << client_fd << " timed out" << std::endl;
		killCgi(client_fd, 504);
	}
}
