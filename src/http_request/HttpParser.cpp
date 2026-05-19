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

void HttpParser::validateRequest()
{
	this->_status = COMPLETE;
}

void HttpParser::validateHeaderHost(const std::string& value)
{
	(void)value;
}

void HttpParser::validateHeaderContentLen(const std::string& value)
{
	(void)value;
}

void HttpParser::parseRequestLine(const std::string& line)
{
	if (!std::regex_match(line, REGEX_REQUEST_LINE))
	{
		_status = ERROR;
		return ;
	}

	std::stringstream	ss(line);
	std::string			raw_path;

	ss >> _request._method;
	ss >> raw_path;
	ss >> _request._version;

	size_t pos = raw_path.find("?");
	if (pos == std::string::npos)
		_request._path = raw_path;
	else
	{
		_request._path = raw_path.substr(0, pos);
		_request._query = raw_path.substr(pos + 1);
	}
}

void HttpParser::parseHeader(const std::string& line)
{
	size_t pos = line.find(":");
	// skip insert in header map when no delemiter or no value after delemiter
	if (pos == std::string::npos || pos == line.length() - 1)
		return ;
	
	std::string key = line.substr(0, pos);
	std::string val = line.substr(pos + 1);

	for (char& c : key)
	{
		c = std::tolower(c);
	}

	// trim optional white space at begin and end of value
	while (val[0] == ' ')
		val.erase(0, 1);
	while (val[val.length() - 1] == ' ')
		val.erase(val.length() - 1, 1);

	// validate important headers
	if (key == "host")
		validateHeaderHost(val);
	else if (key == "content-length")
		validateHeaderContentLen(val);
	
	if (_status == ERROR)
		return ;
	
	_request._headers[key] = val;
}

void HttpParser::parseBuffer()
{
	std::string line;

	if (_state == REQUEST_LINE)
	{
		if (!extractLine(_raw_buffer, line))
			return ;
		parseRequestLine(line);
		if (getStatus() == ERROR)
			return ;
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
				{
					_state = BODY_CONTENT_LEN;
					_content_len_expected = std::stoull(_request._headers["content-length"]);
					if (_content_len_expected <= 0)
						_state = DONE;
				}
				else
					_state = DONE;
				break ;
			}
			parseHeader(line);
			if (_status == ERROR)
				return ;
		}
	}

	if (_state == BODY_CHUNKED)
	{
		// todo
	}

	if (_state == BODY_CONTENT_LEN)
	{
		if (_raw_buffer.size() < _content_len_expected)
			return ;
		_request._body = _raw_buffer.substr(0, _content_len_expected);
		_raw_buffer.erase(0, _content_len_expected);
		_state = DONE;
	}

	if (_state == DONE)
		this->validateRequest();
}

ParseStatus HttpParser::feed(const char *data, size_t n)
{
	_raw_buffer.append(data, n);
	
	// printRawBuffer();

	this->parseBuffer();

	printRequest();

	return (this->getStatus());
}

void HttpParser::printRawBuffer() const
{
	std::cout << "------------ START RawBuffer ------------\n" << _raw_buffer << "------------ END RawBuffer ------------\n";
}

void HttpParser::printRequest() const
{
	std::cout << "------------ REQUEST START ------------\n";
	std::cout << "Method -> [" << _request._method << "]" << std::endl;
	std::cout << "Path   -> [" << _request._path << "]" << std::endl;
	std::cout << "Query  -> [" << _request._query << "]" << std::endl;
	std::cout << "Version-> [" << _request._version << "]" << std::endl;
	for (auto& it : _request._headers)
	{
		std::cout << "Header -> [" << it.first << "] = [" << it.second << "]" << std::endl;
	}
	std::cout << "Body   -> [" << _request._body << "]" << std::endl;
	std::cout << "------------ REQUEST END ------------\n";
}
