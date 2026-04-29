#pragma once

#include "ServerConfig.hpp"

struct Config
{
	std::vector<ServerConfig>	_servers;
	size_t 						_client_header_buffer_size_kb; // 1k;
   	size_t 						_client_timeout_sec; //50s;
	size_t 						_client_max_body_size_mb; //50M;
};
