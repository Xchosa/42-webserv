#pragma once

#include <string>
#include <map>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

#include "utils.hpp"

// function object
struct CaseInsenitiveLess
{
	bool operator()(const std::string& a, const std::string &b) const
	{
		return (lowercase(a) < lowercase(b));
	}
};

struct HttpResponse {
	int														_status_code = 0;
	std::string												_status_text;
	std::string												_version;
	std::map<std::string, std::string, CaseInsenitiveLess>	_headers;
	std::string												_body;

	std::string serialize() const;	// converts HttpResponse in string for _response_buffer in ClientInfos
};
