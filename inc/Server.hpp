#pragma once

#include <sys/epoll.h>
#include <exception>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <string>
#include <iostream>
#include <fcntl.h>
#include <map>

#include "ClientInfos.hpp"
#include "Config.hpp"


inline constexpr size_t MAXEVENTS = 64;
inline constexpr int TIMEOUT = 10;

class Server
{
	private:
		Config							_config;		// alle server configs
		int 							_epoll_fd;		// fd von epoll
		std::map<int, ListenContext>	_socket_fds;	// key = server_fd, value ListenContext*
		std::map<int, ClientInfos>		_clients;		// einzelner client lebt von accept() bis close() bevor er wieder aus der map entfernt wird
	
		void			setNonBlocking(int server_fd);
		void			addFdEpoll(int fd, uint32_t events);
		void			modifyFdEpoll(int fd, uint32_t events);
		void			removeFdEpoll(int fd);
		int				createListeningSocket(const ServerConfig& server_config);
		void			setupListeningSockets();
		void			acceptClient(int server_fd);
		void 			recvClientData(int client_fd);
		void 			sendToClient(int client_fd);
		std::string		normalizeListenHost(const std::string& host);
		std::string		makeListenKey(const ServerConfig& server_config);
		ListenContext 	getOrCreateListenContext(std::map<std::string, ListenContext>& contexts_by_listen, ServerConfig* server_config);
		void 			checkHostWithSamePort(std::map<std::string, ListenContext>& contexts_by_listen, ServerConfig* server_config);
		
		bool			isServerFd(int fd) const;
		void 			closeClient(int client_fd);

	public:
		// OCF
		Server() = delete;
		Server(const Config& config);					// constructor initialisiert _config und _epoll_fd via epoll_create1()
		Server(const Server& other) = delete;
		Server& operator=(const Server& other) = delete;
		~Server();										// destructor muss epoll und alle client_fds closen

		void run();

};
