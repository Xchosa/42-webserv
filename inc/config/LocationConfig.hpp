#pragma once

#include <string>
#include <vector>
#include <optional>
#include <map>

struct LocationConfig
{
	std::string							_name;

	std::string 						_root;
	std::string							_index;
	std::vector<std::string> 			_methods;
	bool								_autoindex		= false;

	std::optional<int>					_redirect_code;
	std::optional<std::string>			_redirect_url;

	bool								_upload			= false;
	std::map<std::string, std::string> 	_cgi_map;
};
