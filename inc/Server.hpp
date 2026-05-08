#pragma once

#include "ClientInfos.hpp"
#include "Config.hpp"


#define MaxEvents 64

class Server
{
	private:
		Config							_config;		// alle server configs
		int 							_epoll_fd;		// fd von epoll
		std::map<int, ListenContext*>	_socket_fds;	// alle socket_fds (unique ports = fuer jeden port 1 socket)
		std::map<int, ClientInfos>		_clients;		// einzelner client lebt von accept() bis close() bevor er wieder aus der map entfernt wird
	
		void addFdEpoll(int fd, uint32_t events);
		void setNonBlocking(int server_fd);
		void modifyFdEpoll(int fd, uint32_t events);
		void removeFdEpoll(int fd);
		int	createListeningSocket(const ServerConfig& server_config);
		void setupListeningSockets();
		void acceptFd(int client_fd);
		void readFd(int client_fd);
		void writeFD(int client_fd);
	
	public:
		// OCF
		Server() = delete;
		Server(const Config& config);					// constructor initialisiert _config und _epoll_fd via epoll_create1()
		Server(const Server& other) = delete;
		Server& operator=(const Server& other) = delete;
		~Server();										// destructor muss epoll und alle client_fds closen

		void run();
		// member functions
		// methoden wie: run(), accept_client(), handlRecv(), handleSend(), usw...
};
