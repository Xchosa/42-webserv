#include "HttpParser.hpp"
#include "ServerConfig.hpp"
#include "ListenContex.hpp"

struct ClientInfos
{
	HttpParser		_parser;		// geparster http request unter _parser._request
	ServerConfig*	_server_config;
	std::string		_response;		// spaeter eigenes struct/ class hier?
};

// instead tore
// which listening socket accepted the client
// pointer to listen group
// only later assign final ServerConfig

struct ClientInfos
{
	int 			_fd;
    ListenContext	*_listen_context;
    ServerConfig	*_selected_server; // Null until header parsed
    HttpParser		_parser;
    HttpRequest		_request;
    std::string		_send_buffer;
    bool 			_request_ready;
    bool 			_response_ready;
    bool 			_should_close;
};