#pragma once

#include <ctime>

#include "HttpParser.hpp"
#include "ServerConfig.hpp"
#include "ListenContext.hpp"
#include "HttpResponse.hpp"

struct ClientInfos
{
    HttpParser		_parser;						// geparster http request unter _parser._request
    ListenContext	*_listen_context;				// possible server_configs in candidates
    ServerConfig	*_selected_server = nullptr;	// null until header parsed
    time_t          _last_activity;
    
	HttpResponse	_response;
	std::string		_response_buffer;				// ganzer response, inkl header

    void	selectVirtualHost();
};
