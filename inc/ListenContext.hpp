#pragma once

#include <string>
#include <vector>
#include "ServerConfig.hpp"

//struct ListenContext {
//    int 						_fd;				// socket_fd
//    std::string					_host;				// bind adress
//    int							_port;				// bind adress
//    std::vector<ServerConfig*>	_candidates;		// alle moeglichen server_configs (gleicher port)
//    ServerConfig*				_default_server;	// erster server_block in _candidates
//};



struct ListenContext {
    int 						_fd;				// socket_fd
    std::string					_host;				// bind adress
    int							_port;				// bind adress
    std::map< std::string, ServerConfig*>	_candidates;		// alle moeglichen server_configs (gleicher port) // dann nach passenden port suchen ->sonst mit iter 
    ServerConfig*				_default_server;	// erster server_block in _candidates
};

// doppelte servernamen verbieten 
// if error page nicht gesetzt -> default error -> option class -> ist gesetzt