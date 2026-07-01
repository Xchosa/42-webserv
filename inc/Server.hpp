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
#include <csignal>
#include <signal.h>

#include "ClientInfos.hpp"
#include "Config.hpp"
#include "Dispatcher.hpp"


inline constexpr size_t MAXEVENTS = 64;

inline constexpr int IDLE_TIME = 5;				// how long epoll_wait blocks until it go further
inline constexpr int KEEP_ALIVE_TIMEOUT = 45;
inline constexpr int CGI_TIMEOUT = 30;
inline constexpr int CHECK_FOR_TIMEOUTS = 5;


class Server
{
	private:
		Config							_config;				// alle server configs
		int 							_epoll_fd;				// fd von epoll
		std::map<int, ListenContext>	_socket_fds;			// key = server_fd, value ListenContext*
		std::map<int, ClientInfos>		_clients;				// einzelner client lebt von accept() bis close() bevor er wieder aus der map entfernt wird
		Dispatcher						_dispatcher;
		std::map<int, int>				_cgi_fd_client_owner;	// haelt fest welcher pipe fd zu welchem client gehoert, gefuellt in recvClientData()
		time_t							_last_timeout_check;
	
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
		ListenContext& 	getOrCreateListenContext(std::map<std::string, ListenContext>& contexts_by_listen, ServerConfig* server_config);
		void 			checkHostWithSamePort(std::map<std::string, ListenContext>& contexts_by_listen, ServerConfig* server_config);
		
		bool			isServerFd(int fd) const;
		bool			isCgiPipeFd(int fd) const;
		void 			closeClient(int client_fd);
		void			killCgi(int client_fd, int error_code);

		void			checkClientTimeouts();
		int				checklastActivity(int client_fd);
		void			checkCgiTimeouts();

		// cgi handling
		void			handleCgiEvent(int pipe_fd, uint32_t event_flag);
		void			cgiWriteBody(int pipe_fd, int client_fd, CgiSession* cgi);
		void			cgiReadOutput(int pipe_fd, int client_fd, ClientInfos* client, CgiSession* cgi);

	public:
		// OCF
		Server() = delete;
		Server(const Config& config);					// constructor initialisiert _config und _epoll_fd via epoll_create1()
		Server(const Server& other) = delete;
		Server& operator=(const Server& other) = delete;
		~Server();										// destructor muss epoll und alle client_fds closen

		void run();

};

	void signalHandler(int sig);
	void initSignal(void);
