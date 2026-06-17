#include "HttpStatus.hpp"

const std::string& getMimeType(std::string& suffix)
{
	static const std::string unknown = "application/octet-stream";

	static const std::map<std::string, std::string> mime_types = {
		{".txt", "text/plain"},
		{".html", "text/html"},
		{".css", "text/css"},
	};

	auto it = mime_types.find(suffix);
	if (it != mime_types.end())
		return (it->second);
	else
		return (unknown);
}
