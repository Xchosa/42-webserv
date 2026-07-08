#include "HttpParser.hpp"

ParseStatus HttpParser::getStatus() const
{
	return (this->_status);
}

void HttpParser::reset()
{
	_raw_buffer.clear();
	_status = INCOMPLETE;
	_state = REQUEST_LINE;
	_content_len_expected = 0;
	_client_server_config = nullptr;
	_total_header_size = 0;
	
	_request._method.clear();
	_request._path.clear();
	_request._query.clear();
	_request._version.clear();
	_request._headers.clear();
	_request._body.clear();
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

void HttpParser::setServerConfig(ServerConfig* conf)
{
	_client_server_config = conf;
}

void HttpParser::validateRequest()
{
	if (!_request._headers.count("host"))
		this->_status = ERROR_400;
	else
		this->_status = COMPLETE;
}

void HttpParser::validateHeaderHost(const std::string& value)
{
	auto pos = value.find_first_of(FORBIDDEN_HOST_CHARS);
	if (pos != std::string::npos)
		_status = ERROR_400;
}

void HttpParser::validateHeaderContentLen(const std::string& value)
{
	size_t idx;
	if (value[0] == '-' || value[0] == '+')
	{
		_status = ERROR_400;
		return ;
	}
	try
	{
		std::stoull(value, &idx);
	}
	catch(const std::exception& e)
	{
		_status = ERROR_400;
	}
	if (idx != value.length())
		_status = ERROR_400;
}

// true on success, false on error
bool HttpParser::decodePath(std::string& path)
{
	if (path.find("%") == std::string::npos) // no encoded chars found, nothing to do here
		return (true);

	std::string decoded;
	decoded.reserve(path.size());

	for (size_t i = 0; i < path.size(); i++)
	{
		if (path[i] != '%')
		{
			decoded += path[i];
			continue;
		}
		if (i + 2 >= path.size()) // not enough chars left
			return (false);
		
		unsigned char byte;
		auto result = std::from_chars(path.data() + i + 1, path.data() + i + 3, byte, 16);
		if (result.ec != std::errc() || result.ptr != path.data() + i + 3)	// not two valid hex chars
			return (false);
		if (byte == 0)
			return (false);	// reject null byte \0
		
		decoded += static_cast<char>(byte);
		i += 2;
	}
	path = decoded;
	return (true);
}

void HttpParser::parseRequestLine(const std::string& line)
{
	if (!std::regex_match(line, REGEX_REQUEST_LINE))
	{
		_status = ERROR_400;
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

	if (decodePath(_request._path) == false)
		_status = ERROR_400;
}

static std::string lowercase(std::string str)
{
	for (auto& c : str)
		c = std::tolower(static_cast<unsigned char>(c));
	return (str);
}

void HttpParser::parseHeader(const std::string& line)
{
	size_t pos = line.find(":");
	// skip insert in header map when no delemiter or no value after delemiter
	if (pos == std::string::npos || pos == line.length() - 1)
		return ;
	
	std::string key = line.substr(0, pos);
	std::string val = line.substr(pos + 1);

	key = lowercase(key);

	// trim whitespaces
	while (!val.empty() && val.back() == ' ')
		val.pop_back();
	while (!val.empty() && val.front() == ' ')
		val.erase(0, 1);

	// validate important headers
	if (key == "host")
		validateHeaderHost(val);
	else if (key == "content-length")
		validateHeaderContentLen(val);
	
	if (_status == ERROR_400)
		return ;
	
	if (!val.empty())
		_request._headers[key] = val;
}



ParseStatus HttpParser::parseChunkedBody()
{
	while(true)
	{	
		//std::cout << "debugging chunked" <<std::endl;
		std::size_t posCRLF = _raw_buffer.find("\r\n");
		if (posCRLF == std::string::npos)
		{ 
			//std::cout << "empty _rawbuffer" <<std::endl;
			_status = INCOMPLETE;
			return(this->getStatus());
		}
		
		std::string ChunkHexSize= _raw_buffer.substr(0, posCRLF);
		//std::cout << "chunkHex: [ "<<  ChunkHexSize << "]" <<std::endl;
		std::size_t parsedChars = 0;
		unsigned long ChunkSizeDez = 0;
		try
		{
			for(size_t i = 0; i < ChunkHexSize.length() ; ++i )
			{
				if (!std::isxdigit(ChunkHexSize[i]))
				{
					//std::cout << "invalid Hex: "  << ChunkHexSize[i] <<std::endl;
					throw std::runtime_error("Invalid hexadecimal character in chunk size");
				}
			}
			ChunkSizeDez = std::stoul( ChunkHexSize, &parsedChars, 16);
		}
		catch(const std::exception& e)
		{
			_status = ERROR_400;
			return(this->getStatus());
		}
		//std::cout << "DezSize: " << ChunkSizeDez <<std::endl;
        if(parsedChars != ChunkHexSize.length())
        {
            _status = ERROR_400;
			//throw std::runtime_error("Invalid hexadecimal character in chunk size");
            return(this->getStatus());
        }
		if (ChunkSizeDez == 0)
		{
			if (_client_server_config->_client_max_body_size != 0 && _request._body.size() > _client_server_config->_client_max_body_size)
			{
				_status = ERROR_413;
				return(this->getStatus());
			}
			if(_raw_buffer.size() < posCRLF + 4)
			{
				_status = INCOMPLETE;
				return(this->getStatus());
			}
			if ( _raw_buffer.substr(posCRLF + 2, 2) != "\r\n")
			{
				_status = ERROR_400;
				return(this->getStatus());
			}

			_raw_buffer.erase(0, posCRLF + 4);
			_state = DONE;
			return(this->getStatus());
        }
		size_t neededSubstr = posCRLF+ 2 + ChunkSizeDez + 2;
		if(neededSubstr > _raw_buffer.length())
		{
			_status = INCOMPLETE;
			return(this->getStatus());
		}
		if(_raw_buffer.substr(posCRLF + 2+ ChunkSizeDez, 2) != ("\r\n"))
		{
			_status = ERROR_400;
			return(this->getStatus());
		}
		if(_client_server_config->_client_max_body_size != 0 && _request._body.length() + ChunkSizeDez > _client_server_config->_client_max_body_size)
		{
			_status = ERROR_413;
			return(this->getStatus());
		}
		_request._body += _raw_buffer.substr(posCRLF + 2, ChunkSizeDez);
		_raw_buffer = _raw_buffer.erase(0,posCRLF + 2 + ChunkSizeDez + 2);
	}
	return _status;
}

ParseStatus HttpParser::parseBuffer()
{
	std::string line;

	if (_status == ERROR_400 || _status == ERROR_413)
		return (this->getStatus());

	if (_state == REQUEST_LINE)
	{
		if (!extractLine(_raw_buffer, line))
			return (this->getStatus());
		parseRequestLine(line);
		if (getStatus() == ERROR_400)
			return (this->getStatus());
		_state = HEADERS;
	}

	// parse header
	if (_state == HEADERS)
	{
		while (extractLine(_raw_buffer, line))
		{
			if (line.empty()) // \r\n\r\n -> headers completed
			{
				_status = HEADER_COMPLETE;

				// choose what happens next
				if (_request._headers.count("transfer-encoding") && lowercase(_request._headers["transfer-encoding"]) == "chunked")
				{
					_state = BODY_CHUNKED;
					return (this->getStatus());
				}
				else if (_request._headers.count("content-length"))
				{
					_state = BODY_CONTENT_LEN;
					_content_len_expected = std::stoull(_request._headers["content-length"]);
					if (_content_len_expected <= 0)
					{
						_state = DONE;
						break ;
					}
					return (this->getStatus());
				}
				else
					_state = DONE;
				break ;
			}
			parseHeader(line);
			if (_status == ERROR_400)
				return (this->getStatus());
		}
	}
	if (_state == BODY_CHUNKED)
	{
		if (_client_server_config == nullptr)
		{
			_status = ERROR_400;
			return (this->getStatus());
		}
		_status = parseChunkedBody();
	}
	
	if (_state == BODY_CONTENT_LEN)
	{
		if (_client_server_config == nullptr)
		{
			_status = ERROR_400;
			return (this->getStatus());
		}
		if (_client_server_config->_client_max_body_size != 0 && _content_len_expected > _client_server_config->_client_max_body_size)
		{
			_status = ERROR_413;
			return (this->getStatus());
		}
		if (_raw_buffer.size() < _content_len_expected)
			return (this->getStatus());
		_request._body = _raw_buffer.substr(0, _content_len_expected);
		_raw_buffer.erase(0, _content_len_expected);
		_state = DONE;
	}

	if (_state == DONE)
		this->validateRequest();

	return (this->getStatus());
}

void HttpParser::feedBuffer(const char *data, size_t n)
{
	_raw_buffer.append(data, n);
	
	// count and check total header size
	if (_state == REQUEST_LINE || _state == HEADERS)
	{
		_total_header_size += n;
		if (_total_header_size > MAX_HEADER_SIZE)
			_status = ERROR_400;
	}
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



const HttpRequest& HttpParser::getRequest() const 
{
	return (this->_request);
}
