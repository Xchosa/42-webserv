#pragma once

#include <string>
#include <map>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

struct HttpResponse {
	int									_status_code = 0;
	std::string							_status_text;
	std::string							_version;
	std::map<std::string, std::string>	_headers;
	std::string							_body;

	std::string serialize() const;	// baut HttpResponse in ein string fuer _response_buffer in ClientInfos
};
