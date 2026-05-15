#include "Server.hpp"
#include <arpa/inet.h>


std::string Server::normalizeListenHost(const std::string& host)
{
	if (host.empty())
		return ("0.0.0.0");
	return (host);
}

std::string Server::makeListenKey(const ServerConfig& server_config)
{
	return (normalizeListenHost(server_config._listen_host) + ":" + std::to_string(server_config._listen_port));
}

void Server::setNonBlocking(int server_fd)
{
	int flags = fcntl(server_fd, F_GETFL, 0);
	if (flags == -1)
	{
		throw std::runtime_error("failed to set server_fd to non_blocking");
	}
	if (fcntl(server_fd, F_SETFL, flags | O_NONBLOCK) == -1)
  		throw std::runtime_error("failed to set fd non_blocking");

}

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
	if (normalizeListenHost(server_config._listen_host) == "0.0.0.0")
		addr.sin_addr.s_addr = INADDR_ANY;
	else if (inet_pton(AF_INET, server_config._listen_host.c_str(), &addr.sin_addr) != 1)
	{
		close(server_fd);
		throw std::runtime_error("invalid listen host");
	}
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


void Server::checkHostWithSamePort(std::map<std::string, ListenContext*>& contexts_by_listen, ServerConfig* server_config)
{
	// new serverConfig 
	std::string host = normalizeListenHost(server_config->_listen_host);
	int port = server_config->_listen_port;
	std::string wildcard_key = "0.0.0.0:" + std::to_string(port);

	// if new host is specific e.g. 127.0.0.1:8085 check if wildcard 0.0.0.0:8085 card exists 
	if (host != "0.0.0.0")
	{
		if(contexts_by_listen.find(wildcard_key) != contexts_by_listen.end())
			throw std::runtime_error("Config error: specific listen host conflicts with 0.0.0.0 on same port");
  		return;

	}
	// if new host is wildcard , loop whether any specific host listens on that port. same hosts still work-> get grouped together
	for (const auto& entry : contexts_by_listen)
  	{
  		ListenContext* context = entry.second;

  		if (context->_port == port && context->_host != "0.0.0.0")
  			throw std::runtime_error("Config error: 0.0.0.0 conflicts with specific listen host on same port");
  	}
}


ListenContext* Server::getOrCreateListenContext(
  	std::map<std::string, ListenContext*>& contexts_by_listen,
  	ServerConfig* server_config
  )
{
	//_listen_host +:+_listen_port = 0.0.0.0:8081
	std::string listen_key = makeListenKey(*server_config);


  	if (contexts_by_listen.find(listen_key) == contexts_by_listen.end())
  	{
  		ListenContext* context = new ListenContext();
  		context->_fd = -1;
  		context->_port = server_config->_listen_port;
  		context->_host = normalizeListenHost(server_config->_listen_host);
  		context->_default_server = server_config;

  		contexts_by_listen[listen_key] = context;
  	}
	checkHostWithSamePort(contexts_by_listen, server_config);

  	return contexts_by_listen[listen_key];
}


void Server::setupListeningSockets()
{
		// 0.0.0.0:9091 => key
	std::map<std::string, ListenContext*> contexts_by_listen;

	for (size_t i = 0; i < _config._servers.size(); ++i)
	{
		ServerConfig* server_config = &_config._servers[i];
		ListenContext* context = getOrCreateListenContext(contexts_by_listen, server_config);

		context->_candidates.push_back(server_config);

		if (server_config->_is_default_server)
			context->_default_server = server_config;
	}
	for(auto& [key, serverName]: contexts_by_listen)
	{
		int server_fd = createListeningSocket(*(serverName->_default_server));
		serverName->_fd = server_fd;
		_socket_fds[server_fd] = serverName;
		addFdEpoll(server_fd, EPOLLIN);
		std::cout << "Server Fd: " << serverName << " | Host: " << serverName->_host << " | Port: " << serverName->_port << std::endl;
	}
}

