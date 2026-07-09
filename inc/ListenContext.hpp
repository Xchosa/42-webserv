#pragma once

#include <string>
#include <vector>
#include "ServerConfig.hpp"

struct ListenContext {
    int 						                _fd;						// socket_fd
    std::string					                _host;						// bind adress
    int							                _port;						// bind adress
    std::vector<ServerConfig*>	                _candidates;	            // all possible server_configs (same listen port host combination)
    ServerConfig*				                _default_server;			// default: first server_block in _candidates, but can be overwritten by defaukt_server flag in config
};
