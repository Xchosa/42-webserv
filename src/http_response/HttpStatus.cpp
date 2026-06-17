#include "HttpStatus.hpp"

const std::string& getStatusText(int code)
{
	static const std::map<int, std::string> texts = {
		// ok
		{200, "OK"},
		{204, "No Content"},
		// redirection
		{301, "Moved Permanently"},
		{302, "Found"},
		{303, "See Other"},
		{307, "Temporary Redirect"},
		{308, "Permanent Redirect"},
		// errors
		{400, "Bad Request"},
		{401, "Unauthorized"},
		{403, "Forbidden"},
		{404, "Not Found"},
		{405, "Method Not Allowed"},
		{410, "Gone"},
		{413, "Payload Too Large"},
	};
	static const std::string unknown = "unknown";

	auto it = texts.find(code);
	if (it != texts.end())
		return (it->second);
	else
		return (unknown);
}
