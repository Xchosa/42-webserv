#include "Dispatcher.hpp"

HttpResponse Dispatcher::dispatch(const HttpRequest& request, ServerConfig* sc)
{
	// find location with longest path match
	// LocationConfig* lc = nullptr;
	// std::cout << std::to_string(sc->_locations.size()) << std::endl;
	// for (auto& it : sc->_locations)
	// {
	// 	(void)it;
	// 	// if (request._path.find(it.first) != std::string::npos)
	// 	// {
	// 	// 	std::cout << "gefunden!\n";
	// 	// }
	// 	// else
	// 	// {
	// 	// 	std::cout << "nicht gefunden!\n";
	// 	// }
	// }


	// find correct handler
	// ...

	// find correct location
	// ...

	(void)sc;
	(void)request;
	HttpResponse dummy;
	return dummy;
}
