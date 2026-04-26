
#include <string>
#include <vector>
#include "Config.hpp"

struct ListenContext {
    int 				_fd;
    std::string			_ip;
    int					_port;
    std::vector<ServerConfig*> _candidates;
    ServerConfig* _default_server;
};


/// not contain 
//socket fd
//epoll event state
//connected clients