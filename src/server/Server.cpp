#include "Server.hpp"



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



//browser connects
//server accepts
//server receives raw HTTP request
//server prints request
//server sends fixed dummy HTTP response
//browser displays page



////int _epoll_fd = epoll_create1(_socket_fds.size());								// fd von epoll
	//std::map<int, ListenContext *> _socket_fds; // alle socket_fds (unique ports = fuer jeden port 1 socket)
	//std::map<int, ClientInfos> _clients;
void Server::run()
{
	//configserver_fd;
	// set up listining socetcs 
	setupListeningSockets();

	epoll_event triggeredEvents[MAXEVENTS]; // size of events 
	
	while(true)
	{
		int readyEvents = epoll_wait(this->_epoll_fd, triggeredEvents, MAXEVENTS, TIMEOUT ); // nbr of events(for each client) retured , cut of by max events
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
				close(fd);// find error client
				continue;
			}
			if (this->_socket_fds.find(fd) != _socket_fds.end())
				acceptFd(fd); // fd = serverfd add new client // new cliend_fd lifes
		
			else if(event_flag & EPOLLIN)
				readFd(fd); // 
			
			else if (event_flag & EPOLLOUT)
				writeFD(fd);

			// send
			

		}
	}



}