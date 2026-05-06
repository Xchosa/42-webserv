#pragma once

#include "LocationConfig.hpp"

// changed _location vector to map 
struct ServerConfig
{
	std::string								_listen_host;
	size_t									_listen_port;
	std::vector<std::string>				_server_names;
	size_t									_client_max_body_size	= 1024 * 1024;	// Angabe in byte // default 1MB
	std::map<int, std::string>				_error_pages;
	bool									_is_default_server;
	std::optional<std::string>				_tmp_root;									// wird nur im parser genutzt!
	std::optional<std::string>				_tmp_index;									// wird nur im parser genutzt!
	std::map< std::string, LocationConfig>	_locations;

	//helper methods
	// ...
};
