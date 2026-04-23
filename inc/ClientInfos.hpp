#include "HttpParser.hpp"
#include "ServerConfig.hpp"

struct ClientInfos
{
	HttpParser		_parser;		// geparster http request unter _parser._request
	ServerConfig*	_server_config;
	std::string		_response;		// spaeter eigenes struct/ class hier?
};
