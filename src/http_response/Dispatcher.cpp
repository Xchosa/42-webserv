#include "Dispatcher.hpp"

LocationConfig* Dispatcher::findLocation(const std::string& path, ServerConfig* sc) const
{
	LocationConfig*	lc = nullptr;
	size_t			longest_match = 0;

	for (auto& it : sc->_locations)
	{
		// std::cout << "search for: '" << it.first << "' in '" << path << "'\n";
		if (path.find(it.first) != std::string::npos)
		{
			size_t location_len = it.first.length();
			if (location_len > longest_match)
			{
				longest_match = location_len;
				lc = &it.second;
				// std::cout << "'" << it.first << "' ist neue location\n";
			}
		}
	}
	return (lc);
}

HttpResponse Dispatcher::dispatch(const HttpRequest& request, ServerConfig* sc)
{
	// find location with longest path match
	LocationConfig*	lc = this->findLocation(request._path, sc);
	if (lc == nullptr)
	{
		// 404 error werfen
	}

	// find correct handler
	// ...

	// find correct location
	// ...

	HttpResponse dummy;
	return dummy;
}
