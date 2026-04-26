#include "ClientInfos.hpp"
#include "Config.hpp"

//class Server
//{
//private:
//	Config &_config;
//	int _epoll_fd;
//	std::map<int, ServerConfig *> _server_fds;
//	std::map<int, ClientInfos> _clients; // einzelner client lebt von accept() bis close() bevor er wieder aus der map entfernt wird

//public:
//	// methoden wie: run(), accept_client(), handlRecv(), handleSend(), usw...
//};

class Server
{
	private:
		Config _config;
		int _epoll_fd;
		std::map<int, ListenContext *> _listen_fds;
		std::map<int, ClientInfos> _clients;
	public:

};
