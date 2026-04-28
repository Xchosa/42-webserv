#pragma once

#include <string>
#include <vector>
#include "ServerConfig.hpp"

struct ListenContext {
    int 						_fd;				// socket_fd
    std::string					_host;				// bind adress
    int							_port;				// bind adress
    std::vector<ServerConfig*>	_candidates;		// alle moeglichen server_configs (gleicher port)
    ServerConfig*				_default_server;	// erster server_block in _candidates
};
