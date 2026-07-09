#pragma once

#include <ctime>

#include "HttpParser.hpp"
#include "ServerConfig.hpp"
#include "ListenContext.hpp"
#include "HttpResponse.hpp"
#include "CgiSession.hpp"

struct ClientInfos
{
    HttpParser		_parser;						// parsed http request in _parser._request
    ListenContext	*_listen_context;				// possible server_configs in candidates
    ServerConfig	*_selected_server = nullptr;	// null until header parsed
    time_t          _last_activity;
    
	HttpResponse	_response;
	std::string		_response_buffer;				// whole response, incl header to send out to client

	std::optional<CgiSession>	_cgi;

    void	selectVirtualHost();
};
