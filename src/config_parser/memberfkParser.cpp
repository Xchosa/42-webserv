
#include "HttpParser.hpp"

ParseStatus HttpParser::feed(const char *data, size_t n)
{
	_raw_buffer.append(data, n);

	if (_raw_buffer.find("\r\n\r\n") == std::string::npos)
	{
		_status = INCOMPLETE;
		return _status;
	}

	_status = COMPLETE;
	return _status;
}

//std::string HttpParser::TMP_getRawBuffer() const
//{
//	return _raw_buffer;
//}
