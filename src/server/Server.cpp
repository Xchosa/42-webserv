#include "Server.hpp"
#include <iostream>


// constructor initialisiert _config und _epoll_fd via epoll_create1()
Server::Server(const Config &config) : _config(config) , _epoll_fd(epoll_create1(0))
{
	if (_epoll_fd == -1)
		throw std::runtime_error("Failed to create epoll file descriptor");
	
	// alle server configs
	

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

}

//brower connect

//browser connects
//server accepts
//server receives raw HTTP request
//server prints request
//server sends fixed dummy HTTP response
//browser displays page




void Server::run()
{
	setupListeningSockets();
	for (const auto& [fd, context] : _socket_fds)
		std::cout << "Server FD: " << fd << " | Port: " << context._port << std::endl;
	epoll_event triggeredEvents[MAXEVENTS]; // size of events 
	
	while(true)
	{
		int readyEvents = epoll_wait(this->_epoll_fd, triggeredEvents, MAXEVENTS, IDLE_TIME * 1000); // nbr of events(for each client) retured , cut of by max events
		if (readyEvents == -1)
		{
			if (errno == EINTR)
				continue;
			throw std::runtime_error("epoll_wait failed");
		}
		for(int i = 0; i < readyEvents; ++i)
		{
			int fd = triggeredEvents[i].data.fd;
			uint32_t event_flag = triggeredEvents[i].events;

			if(event_flag & (EPOLLERR | EPOLLHUP | EPOLLRDHUP))
			{
				if(isServerFd(fd))
				{
					throw std::runtime_error("listening socket error");
				}
				closeClient(fd);
				continue;
			}
			if (isServerFd(fd))
				acceptClient(fd); // fd = serverfd add new client // new cliend_fd lifes
			else if (event_flag & EPOLLIN)
				recvClientData(fd);
			else if (event_flag & EPOLLOUT)
				sendToClient(fd);
		}
		if(readyEvents == 0)
		{
			checkClientTimeouts();
		}
	}
}
