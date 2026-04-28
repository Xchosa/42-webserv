
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
////int _epoll_fd = epoll_create1(_socket_fds.size());								// fd von epoll
	//std::map<int, ListenContext *> _socket_fds; // alle socket_fds (unique ports = fuer jeden port 1 socket)
	//std::map<int, ClientInfos> _clients;
//void Server::run()
//{

//}