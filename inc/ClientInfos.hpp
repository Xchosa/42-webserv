#pragma once

#include "HttpParser.hpp"
#include "ServerConfig.hpp"
#include "ListenContext.hpp"

struct ClientInfos
{
    HttpParser		_parser;			// geparster http request unter _parser._request
    ListenContext	*_listen_context;	// possible server_configs in candidates
    ServerConfig	*_selected_server;	// null until header parsed
    std::string		_response_buffer;	// ganzer response, inkl header
};
