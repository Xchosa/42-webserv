#include "HttpResponse.hpp"

std::string HttpResponse::serialize() const
{
	std::string buffer;

	// response line
	buffer.append(_version);
	buffer.append(" ");
	buffer.append(std::to_string(_status_code));
	buffer.append(" ");
	buffer.append(_status_text);
	buffer.append("\r\n");

	// header
	for (auto& it : _headers)
	{
		buffer.append(it.first);
		buffer.append(":");
		buffer.append(it.second);
		buffer.append("\r\n");
	}
	buffer.append("\r\n");

	// body
	buffer.append(_body);

	return (buffer);
}
