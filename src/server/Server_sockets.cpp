#include "webserv.hpp"



int Server::createListeningSocket(const ServerConfig& server_config)
{
	int server_fd = socket(AF_INET, SOCK_STREAM, 0 );
	if (server_fd == -1)
		throw std::runtime_error("socket failed");

	int opt = 1; // 0 for disable -> default behaviour 
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		close(server_fd);
		throw std::runtime_error("setsockopt failed");
	}

	sockaddr_in addr{}; 

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY; // listen to all local interfaces -> late to _listen_host
	addr.sin_port = htons(server_config._listen_port);

	if (bind(server_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1)
	{
		close(server_fd);
		throw std::runtime_error("bind failed");
	}

	if (listen(server_fd, SOMAXCONN) == -1) // 
	{
		close(server_fd);
		throw std::runtime_error("listen failed");
	}

	setNonBlocking(server_fd);

	return server_fd;
}

void Server::setupListeningSockets()
{
	for(size_t i = 0; i <= _config._servers.size(); i++)
	{
		int server_fd = createListeningSocket(_config._servers[i]);
		this->_socket_fds[server_fd] = NULL;
		addFdEpoll(server_fd, EPOLLIN);

		std::cout<< "Listening on port: " <<  _config._servers[i]._listen_port << "| fd: " << server_fd << std::endl;



	}
}