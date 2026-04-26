#include "LocationConfig.hpp"

//struct ServerConfig
//{
//	std::string _listen_host;
//	size_t _listen_port;
//	std::string _server_name;
//	size_t _client_max_body_size = 1024 * 1024; // default: 1MB
//	std::map<int, std::string> _error_pages;
//	std::vector<LocationConfig> _locations;

//	// helper methods
//	//  ...
//};

struct ServerConfig
{
	std::vector<std::pair<std::string, int>> _listens; // ("0.0.0.0", 8080)
	std::vector<std::string> 				_server_names;			  // "a.local", "www.a.local"
	size_t 									_client_max_body_size;			 // e.g. 1000000
	std::map<int, std::string>				_error_pages; // 404 -> "/errors/404.html"
	std::vector<LocationConfig> 			_locations;
	bool									_is_default_server; // optional helper
	std::string 							_root;		// default root
	std::string 							_index;		// default index

	ServerConfig()
		: _client_max_body_size( 1024 * 1024),
		  _is_default_server(false)
	{}
};