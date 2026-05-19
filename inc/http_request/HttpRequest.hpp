#pragma once

#include <string>
#include <map>

struct HttpRequest {
	std::string							_method;
	std::string							_path;
	std::string							_query;
	std::string							_version;
	std::map<std::string, std::string>	_headers;
	std::string							_body;
};
