#pragma once

#include "ClientInfos.hpp"
#include "Config.hpp"

class Server
{
	private:
		Config							_config;		// alle server configs
		int 							_epoll_fd;		// fd von epoll
		std::map<int, ListenContext*>	_socket_fds;	// alle socket_fds (unique ports = fuer jeden port 1 socket)
		std::map<int, ClientInfos>		_clients;		// einzelner client lebt von accept() bis close() bevor er wieder aus der map entfernt wird
	public:
		// OCF
		Server() = delete;
		Server(const Config& config);					// constructor initialisiert _config und _epoll_fd via epoll_create1()
		Server(const Server& other) = delete;
		Server& operator=(const Server& other) = delete;
		~Server();										// destructor muss epoll und alle client_fds closen

		// member functions
		// methoden wie: run(), accept_client(), handlRecv(), handleSend(), usw...
};
