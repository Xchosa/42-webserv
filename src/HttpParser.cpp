#include "HttpParser.hpp"

ParseStatus HttpParser::getStatus() const
{
	return (this->_status);
}

bool HttpParser::extractLine(std::string& buffer, std::string& line)
{
	size_t pos = buffer.find("\r\n");
	if (pos == std::string::npos)
		return (false);
	
	line = buffer.substr(0, pos);
	buffer.erase(0, pos + 2);

	return (true);
}

void HttpParser::parseRequestLine(const std::string& line)
{
	std::stringstream ss(line);

	ss >> _request._method;
	ss >> _request._path;
	ss >> _request._version;

	// query noch splitten vom path
}

void HttpParser::parseHeader(const std::string& line)
{
	size_t pos = line.find(":");
	if (pos == std::string::npos)
		return ;
	
	std::string key = line.substr(0, pos);
	std::string val = line.substr(pos + 1);

	for (char& c : key)
	{
		c = std::tolower(c);
	}

	if (val[0] == ' ')
		val.erase(0, 1);
	
	_request._headers[key] = val;
	std::cout << "Header -> [" << key << "] = [" << val << "]" << std::endl;

}

void HttpParser::parseBuffer()
{
	std::string line;

	if (_state == REQUEST_LINE)
	{
		if (!extractLine(_raw_buffer, line))
			return ;
		parseRequestLine(line);
		_state = HEADERS;
	}

	// parse header
	if (_state == HEADERS)
	{
		while (extractLine(_raw_buffer, line))
		{
			if (line.empty()) // \r\n\r\n -> headers completed, choose what happens next
			{
				if (_request._headers.count("transfer-encoding") && _request._headers["transfer-encoding"] == "chunked")
					_state = BODY_CHUNKED;
				else if (_request._headers.count("content-length"))
					_state = BODY_CONTENT_LEN;
				else
					_state = DONE;
				break ;
			}
			parseHeader(line);
		}
	}

	if (_state == BODY_CHUNKED)
	{
		// todo
	}

	if (_state == BODY_CONTENT_LEN)
	{
		// todo
	}

	if (_state == DONE)
		_status == COMPLETE;

	// // parse body
	// if (_request._headers.find("Transfer-Encoding") != _request._headers.end())
	// {
	// 	if (_request._headers["Transfer-Encoding"] == "chunked")
	// 	{
	// 		// get body with chunked blocks
	// 	}
	// }
	// else if (_request._headers.find("Content-Length") != _request._headers.end())
	// {
	// 	// get body with content length
	// }
	// _status = COMPLETE;
}

ParseStatus HttpParser::feed(const char *data, size_t n)
{
	_status = INCOMPLETE;
	_raw_buffer.append(data, n);
	std::cout << TMP_getRawBuffer() << std::endl;	// debugging
	this->parseBuffer();

	// if (_raw_buffer.find("\r\n\r\n") == std::string::npos)
	// {
	// 	_status = INCOMPLETE;
	// 	return _status;
	// }





	return (this->getStatus());
}

std::string HttpParser::TMP_getRawBuffer() const
{
	return _raw_buffer;
}
