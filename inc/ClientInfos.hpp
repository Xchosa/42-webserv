#pragma once

#include "HttpParser.hpp"
#include "ServerConfig.hpp"
#include "ListenContext.hpp"
#include <ctime>

struct ClientInfos
{
    HttpParser		_parser;			// geparster http request unter _parser._request
    ListenContext	*_listen_context;	// possible server_configs in candidates
    ServerConfig	*_selected_server;	// null until header parsed
    std::string		_response_buffer;	// ganzer response, inkl header
    time_t          _last_activity;


    ServerConfig*    selectVirtualHost();

};

// dispeatchure classe -> 3 weitere klassen -> static, cgi, file upload 