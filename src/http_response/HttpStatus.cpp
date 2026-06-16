#include "HttpStatus.hpp"

const std::string& getStatusText(int code)
{
	static const std::map<int, std::string> texts = {
		{200, "OK"},
		{400, "Bad Request"},
		{404, "Not Found"},
		{413, "Payload Too Large"},
	};
	static const std::string unknown = "unknown";

	auto it = texts.find(code);
	if (it != texts.end())
		return (it->second);
	else
		return (unknown);
}
