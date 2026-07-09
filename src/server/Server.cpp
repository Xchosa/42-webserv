#include "Server.hpp"

extern volatile std::sig_atomic_t gSignalStatus;

// constructor initialise _config and _epoll_fd via epoll_create1()
Server::Server(const Config &config) : _config(config) , _epoll_fd(epoll_create1(0))
{
	if (_epoll_fd == -1)
		throw std::runtime_error("Failed to create epoll file descriptor");
}

Server::~Server()
{
	// destroy _epoll instance
	if (_epoll_fd != -1)
	{
		close(_epoll_fd);
	}

	// ListenContext close
	for(const auto &[fd ,ListenContext] : _socket_fds)
	{
		close(fd);
	}
	
	// client sockets
	for (const auto &[fd, ClientInfos] : _clients)
	{
		close(fd);
	}
	std::cout<< "Sockets and epoll Instance cleaned" <<std::endl;

}

void Server::run()
{
	setupListeningSockets();

	// LOGGING
	for (const auto& [fd, context] : _socket_fds)
	{
		bool		first = true;
		std::string	server_names;
		for (ServerConfig* sc : context._candidates)
		{
			for (std::string& name : sc->_server_names)
			{
				if (name.empty())
					continue;
				if (!first)
					server_names += ", ";
				server_names += name;
				first = false;
			}
		}
		std::cout 	<< "listening on " 
					<< context._host << ":" << context._port 
					<< " (fd=" << fd << ", " << context._candidates.size() 
					<< (context._candidates.size() == 1 ? " virtual host" : " virtual hosts")
					<< " [" << server_names << "]"
					<< ")" << std::endl;
	}
	// LOGGING END

	epoll_event triggeredEvents[MAXEVENTS];
	initSignal();
	
	_last_timeout_check = time(nullptr);
	while(gSignalStatus)
	{
		int readyEvents = epoll_wait(this->_epoll_fd, triggeredEvents, MAXEVENTS, IDLE_TIME * 1000);
		if (readyEvents == -1)
		{
			if (errno == EINTR)
				continue;
			throw std::runtime_error("epoll_wait failed");
		}

		time_t now = time(nullptr);
		if ((now - _last_timeout_check) >= CHECK_FOR_TIMEOUTS)
		{
			checkClientTimeouts();
			checkCgiTimeouts();
			_last_timeout_check = now;
		}

		for(int i = 0; i < readyEvents; ++i)
		{
			int fd = triggeredEvents[i].data.fd;;
			uint32_t event_flag = triggeredEvents[i].events;

			if(event_flag & (EPOLLERR | EPOLLHUP | EPOLLRDHUP))
			{
				if(isServerFd(fd))
				{
					throw std::runtime_error("listening socket error");
				}
				else if (!isCgiPipeFd(fd))
				{
					closeClient(fd);
					continue;
				}
			}
			if (isServerFd(fd))
			{
				std::cout << "[INFO]  ServerFd: " << fd << ", accept new client" << std::endl;
				acceptClient(fd); // fd = serverfd add new client // new cliend_fd lifes
			}
			else if (isCgiPipeFd(fd))
			{
				int 			client_fd = _cgi_fd_client_owner[fd];
				ClientInfos*	client = &_clients[client_fd];
				handleCgiEvent(fd, event_flag);
				if (client->_cgi.has_value() && client->_cgi.value()._waited == true)
				{
					modifyFdEpoll(client_fd, EPOLLOUT | EPOLLRDHUP);
					client->_parser.reset();
					if (client->_cgi.value()._stdin_fd != -1)
					{
						int stdin_fd = client->_cgi.value()._stdin_fd;
						removeFdEpoll(stdin_fd);
						close(stdin_fd);
						client->_cgi.value()._stdin_fd = -1;
						_cgi_fd_client_owner.erase(stdin_fd);
					}
					client->_cgi.reset();
				}
			}
			else if (event_flag & EPOLLIN)
			{
				recvClientData(fd);
				ParseStatus status = _clients[fd]._parser.getStatus();
				if ((status == COMPLETE || status == ERROR_400 || status == ERROR_413) && !_clients[fd]._cgi.has_value()) // only send when response ready and no cgi
				{
					modifyFdEpoll(fd, EPOLLOUT | EPOLLRDHUP);
					_clients[fd]._parser.reset();
				}
			}
			else if (event_flag & EPOLLOUT)
			{
				sendToClient(fd);
				if(_clients.count(fd))
					modifyFdEpoll(fd, EPOLLIN | EPOLLRDHUP);
			}
		}
	}
}
