#pragma once

#include "LocationConfig.hpp"

struct ServerConfig
{
	std::string					_listen_host;
	size_t						_listen_port;
	std::vector<std::string>	_server_names;
	size_t						_client_max_body_size	= 1024 * 1024; // default: 1MB
	std::map<int, std::string>	_error_pages;
	std::vector<LocationConfig>	_locations;
	bool						_is_default_server;

	//helper methods
	// ...
};
