#include "ClientInfos.hpp"
#include "Server.hpp"

void ClientInfos::selectVirtualHost()
{
	HttpRequest req	= _parser.getRequest();

	for(auto const&[headerName, headerValue] : req._headers)
	{
		if (headerName == "host")
		{
			for(size_t i = 0; i < _listen_context->_candidates.size(); ++i)
			{
				ServerConfig *tmpServer = _listen_context->_candidates[i];
				if (tmpServer == nullptr)
					continue;
				for (size_t j = 0; j < tmpServer->_server_names.size(); ++j)
				{
					if(headerValue == tmpServer->_server_names[j])
					{
						_selected_server = tmpServer;
						std::cout << "[INFO]  virtual host match: '" << tmpServer->_server_names[j] << "'" << std::endl;
						break;
					}
				}
			}
		}
	}
	if (_selected_server == nullptr)
	{
		_selected_server = _listen_context->_default_server;
		std::cout << "[INFO]  no virtual host match, take default server: '" << _listen_context->_default_server->_server_names[0] << "'" << std::endl;
	}
	_parser.setServerConfig(_selected_server);
}
