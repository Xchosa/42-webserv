#include "Dispatcher.hpp"
#include "HttpException.hpp"
#include "algorithm"

LocationConfig* Dispatcher::findLocation(const std::string& path, ServerConfig* sc) const
{
	LocationConfig*	lc = nullptr;
	size_t			longest_match = 0;

	for (auto& it : sc->_locations)
	{
		const std::string&	cur_location = it.first;
		size_t				cur_location_len = cur_location.length();
		// std::cout << "search for: '" << cur_location << "' in '" << path << "'\n";

		if (path.rfind(cur_location, 0) != 0)
			continue;

		if (path.length() > cur_location_len && path[cur_location_len] != '/')
			continue;

		if (cur_location_len > longest_match)
		{
			longest_match = cur_location_len;
			lc = &it.second;
			// std::cout << "'" << cur_location << "' ist neue location\n";
		}
	}
	return (lc);
}

void Dispatcher::checkMethodAllowed(std::string method, std::vector<std::string> allowed_methods)
{
	auto it = std::find(allowed_methods.begin(), allowed_methods.end(), method);

	if (it == allowed_methods.end())
		throw HttpException(405);
}

HttpResponse Dispatcher::handleRedirect(LocationConfig* lc)
{
	HttpResponse r;

	// TODO: text ist aktuell nur fuer code 301

	r._version = "HTTP/1.1";
	r._status_code = lc->_redirect_code.value();
	r._status_text = "Moved Permanently";
	r._headers["Location"] = lc->_redirect_url.value();
	r._headers["Content-Length"] = "0";

	return (r);
}

HttpResponse Dispatcher::dispatch(const HttpRequest& request, ServerConfig* sc)
{
	try
	{
		// find location with longest path match
		LocationConfig*	lc = this->findLocation(request._path, sc);
		if (lc == nullptr)
			throw HttpException(404);

		if (lc->_redirect_code.has_value())
			return (handleRedirect(lc));

		checkMethodAllowed(request._method, lc->_methods);


		// find correct handler
		// ...

		// find correct location
		// ...
	}
	catch(const HttpException& e)
	{
		// build_error_response(e.code()); // TODO
		std::cerr << e.code() << '\n';
	}

	HttpResponse dummy;
	return dummy;
}
