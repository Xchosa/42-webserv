
#include "webserv.hpp"



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

void Server::setNonBlocking(int server_fd)
{
	int flags = fcntl(server_fd, F_GETFL, 0);
	if (flags == -1)
	{
		throw std::runtime_error("failed to set server_fd to non_blocking");
	}
}


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
		throw std::runtime_error("epoll_ctl DEL failed (fd invalid,notRegistered,closed)")
}







////int _epoll_fd = epoll_create1(_socket_fds.size());								// fd von epoll
	//std::map<int, ListenContext *> _socket_fds; // alle socket_fds (unique ports = fuer jeden port 1 socket)
	//std::map<int, ClientInfos> _clients;
void Server::run()
{
	//configserver_fd;
	// set up listining socetcs 
	setupListeningSockets();

	epoll_event triggeredEvents[MaxEvents]; // size of events 
	
	while(true)
	{
		int readyEvents = epoll_wait(this->_epoll_fd, triggeredEvents ,MaxEvents,10); // nbr of events(for each client) retured , cut of by max events
		if (readyEvents == -1)
		{
			if (errno == EINTR)
				continue;
			throw std::runtime_error("epoll_wait failed");
		}
		for(int i = 0; i <= readyEvents; i++)
		{
			
			// listining, 
		}
	// epoll created in constructor
	}



}