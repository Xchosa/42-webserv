#include "Dispatcher.hpp"

std::string Dispatcher::upperString(std::string str) const
{
	std::transform(str.begin(), str.end(), str.begin(), 
		[](unsigned char c)
		{
			return std::toupper(c);
		}
	);
	return (str);
}

HttpResponse Dispatcher::handleCgi(const HttpRequest& request, ServerConfig* sc, LocationConfig* lc)
{
	std::cout << "> CGI HANDLER\n";
	// get dateiendung und pruefen ob in cgi map vorhanden
	std::string extension;
	if (request._path.find(".") != std::string::npos)
	{
		extension = request._path.substr(request._path.find_last_of('.'));
		auto it = lc->_cgi_map.find(extension);
		if (it == lc->_cgi_map.end())
		{
			std::cout << "extension nicht gefunden in cgi map\n";
			throw HttpException(404); // TODO oder als static file handeln?
		}
	}
	else // keine file angegeben
	{
		std::cout << "keine file angegeben\n";
		throw HttpException(404); // TODO oder als static file handeln?
	}

	// pfad bauen
	std::string script_path = getFullRootPath(lc) + request._path;
	// datei vorhanden und lesbar?
	if (access(script_path.c_str(), F_OK) == -1)
		throw HttpException(404);
	if (access(script_path.c_str(), R_OK) == -1)
		throw HttpException(403);


	// env's bauen
	// env.push_back("=" + );
	std::vector<std::string> env;

	env.push_back("GATEWAY_INTERFACE=CGI/1.1");
	env.push_back("SERVER_PROTOCOL=" + request._version);
	env.push_back("REQUEST_METHOD=" + request._method);
	env.push_back("SCRIPT_NAME=" + request._path);
	env.push_back("SCRIPT_FILENAME=" + script_path);
	env.push_back("SERVER_NAME=" + sc->_listen_host);
	env.push_back("SERVER_PORT=" + std::to_string(sc->_listen_port));

	if (request._query.length() > 0)
		env.push_back("QUERY_STRING=" + request._query);
	if (request._body.length() > 0)
		env.push_back("CONTENT-LENGTH=" + std::to_string(request._body.length()));
	if (extension == ".php" || extension == ".PHP")
		env.push_back("REDIRECT_STATUS=200");
	if (request._headers.find("content-type") != request._headers.end())
		env.push_back("CONTENT_TYPE=" + request._headers.at("content-type"));

	
	// request header
	for (auto &h : request._headers)
	{
		env.push_back("HTTP_" + upperString(h.first) + "=" + h.second);
	}



	std::cout << "env's:\n";
	for (auto &s : env)
	{
		std::cout << "- " << s << std::endl;
	}

	HttpResponse dummy;
	return (dummy);
}
