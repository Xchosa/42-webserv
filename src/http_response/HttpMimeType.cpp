#include "HttpMimeType.hpp"

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

const std::string& getMimeTypeFromFile(std::string& path)
{
	static const std::string unknown = "application/octet-stream";
	size_t dot_pos = path.rfind('.');

	if (dot_pos != std::string::npos)
	{
		std::string suffix = path.substr(dot_pos);
		return (getMimeType(suffix));
	}
	return (unknown);
}
