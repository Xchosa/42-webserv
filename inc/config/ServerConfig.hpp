#include "LocationConfig.hpp"

struct ServerConfig
{
	std::string					listen_host;
	size_t						listen_port;
	std::string					server_name;
	size_t						client_max_body_size	= 1024 * 1024; // default: 1MB
	std::map<int, std::string>	error_pages;
	std::vector<LocationConfig>	locations;

	//helper methods
	// ...
};
