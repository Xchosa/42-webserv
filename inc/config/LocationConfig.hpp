#include <string>
#include <vector>
#include <optional>
#include <map>

struct LocationConfig
{
	std::string							_path;

	std::optional<std::string> 			_root;
	std::optional<std::string> 			_index;
	std::optional<bool>        			_autoindex;
	std::vector<std::string> 			_methods; 

	std::optional<int>					_redirect_code;
	std::optional<std::string>			_redirect_url;
	
	std::optional<std::string>			_upload_store;
	std::optional<size_t>				_max_body_size;
	std::map<std::string, std::string> 	_cgi_map;

	// helper methods
	bool allows_method(const std::string& method) const;
};

// if not set -> optional inherit from ServerConfig 
// if set in location -> overrite server_defaults 