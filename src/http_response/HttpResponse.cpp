#include "HttpResponse.hpp"

static std::string get_http_date(void)
{
	std::time_t t = std::time(nullptr);
	std::tm tm{};
	gmtime_r(&t, &tm); // UTC time, no timezone

	std::ostringstream oss;
	oss << std::put_time(&tm, "%a, %d %b %Y %H:%M:%S GMT");
	return (oss.str());
}

std::string HttpResponse::serialize() const
{
	std::string buffer;

	// response line
	buffer.append(_version);
	buffer.append(" ");
	if (_status_code > 0)
		buffer.append(std::to_string(_status_code));
	buffer.append(" ");
	buffer.append(_status_text);
	buffer.append("\r\n");

	// header
	for (auto& it : _headers)
	{
		buffer.append(it.first);
		buffer.append(": ");
		buffer.append(it.second);
		buffer.append("\r\n");
	}

	// date header
	buffer.append("Date: ");
	buffer.append(get_http_date());
	buffer.append("\r\n");

	// end header
	buffer.append("\r\n");

	// body
	buffer.append(_body);

	return (buffer);
}
