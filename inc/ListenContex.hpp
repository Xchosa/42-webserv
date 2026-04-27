#pragma once

#include <string>
#include <vector>
#include "ServerConfig.hpp"

struct ListenContext {
    int 						_fd;
    std::string					_host; // ip
    int							_port;
    std::vector<ServerConfig*>	_candidates;
    ServerConfig*				_default_server;
};
