#include "ClientInfos.hpp"
#include "Server.hpp"

void ClientInfos::selectVirtualHost()
{
	HttpRequest req	= _parser.getRequest();

	for(auto const&[headerName, headerValue] : req._headers)
	{
		if (headerName == "host")
		{
			std::cout << "header key   = [" << headerName << "]\n";
			std::cout << "header value = [" << headerValue << "]\n";
			
			for(size_t i = 0; i < _listen_context->_candidates.size(); ++i)
			{
				ServerConfig *tmpServer = _listen_context->_candidates[i];
				if (tmpServer == nullptr)
				{
					std::cout << "  tmpServer is NULL" << std::endl;
					continue;
				}

				std::cout << "  listen host: " << tmpServer->_listen_host << std::endl;
				std::cout << "  listen port: " << tmpServer->_listen_port << std::endl;
				std::cout << "  listen server: " <<  std::endl;
				for (size_t j = 0; j < tmpServer->_server_names.size(); ++j)
				{
					std::cout << "    [" << tmpServer->_server_names[j] << "]" << std::endl;
					if(headerValue == tmpServer->_server_names[j])
					{
						std::cout << "match found: " <<  tmpServer->_server_names[j] <<std::endl;
						_selected_server = tmpServer;
						break;
					}
				}
			}
		}
	}
	if (_selected_server == nullptr)
	{
		std::cout << "kein server gefunden, nehme default\n";
		_selected_server = _listen_context->_default_server;
	}
	_parser.setServerConfig(_selected_server);
}
