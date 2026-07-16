#include "Dispatcher.hpp"

std::string Dispatcher::upperString(std::string str) const
{
	for (char &c : str)
	{
		if (c == '-')
			c = '_';
		else
			c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
	}
	return (str);
}

void Dispatcher::buildEnv(std::vector<std::string>& env, const HttpRequest& request, std::string& path, std::string& script_path, ServerConfig* sc)
{
	env.push_back("GATEWAY_INTERFACE=CGI/1.1");
	env.push_back("SERVER_PROTOCOL=" + request._version);
	env.push_back("REQUEST_METHOD=" + request._method);
	env.push_back("SCRIPT_NAME=" + path);
	env.push_back("SCRIPT_FILENAME=" + script_path);
	env.push_back("SERVER_PORT=" + std::to_string(sc->_listen_port));
	env.push_back("QUERY_STRING=" + request._query);
	if (sc->_listen_host.length() > 0)
		env.push_back("SERVER_NAME=" + sc->_listen_host);
	if (request._body.length() > 0)
		env.push_back("CONTENT_LENGTH=" + std::to_string(request._body.length()));
	if (upperString(path.substr(path.find_last_of('.'))) == ".PHP")
		env.push_back("REDIRECT_STATUS=200");
	if (request._headers.find("content-type") != request._headers.end())
		env.push_back("CONTENT_TYPE=" + request._headers.at("content-type"));

	// add request header
	for (auto &h : request._headers)
	{
		if (h.first == "content-type" || h.first == "content-length")
			continue;
		env.push_back("HTTP_" + upperString(h.first) + "=" + h.second);
	}
}

HttpResponse Dispatcher::parseCgiOutput(std::string& output)
{
	int skip = 4;
	size_t header_end = output.find("\r\n\r\n");
	if (header_end == std::string::npos)
	{
		skip = 2;
		header_end = output.find("\n\n");
	}
	if (header_end == std::string::npos)
		throw HttpException(502);
	std::string headers = output.substr(0, header_end + (skip / 2));
	std::string	body = output.substr(header_end + skip);

	HttpResponse r;
	r._version = "HTTP/1.1";
	r._status_code = 200;
	r._headers["Content-Type"] = "application/octet-stream";

	// parse headers
	std::string	delimiter = skip == 4 ? "\r\n" : "\n";
	size_t		pos;
	std::string	line;
	while ((pos = headers.find(delimiter)) != std::string::npos)
	{
		// extract line
		line = headers.substr(0, pos);
		headers.erase(0, pos + delimiter.length());

		if (line.empty())
			continue;

		// parsing
		size_t p = line.find(":");
		if (p != std::string::npos)
		{
			std::string key = line.substr(0, p);
			std::string val = line.substr(p + 1);
			// std::cout << "[DEBUG] CGI key-value: " << key << "[" << val << "]" << std::endl;

			// trim whitespaces
			while (!val.empty() && val.back() == ' ')
				val.pop_back();
			while (!val.empty() && val.front() == ' ')
				val.erase(0, 1);

			// skip if value empty
			if (val.empty())
				continue;

			if (lowercase(key) == "status")
			{
				int code;
				try
				{
					code = std::stoi(val);
				}
				catch(const std::exception& e)
				{
					code = 200;
				}
				if (code < 100 || code > 999)
					code = 200;
				r._status_code = code;
			}
			else if (lowercase(key) == "set-cookie")
			{
				size_t cookie_equal_sign = val.find("=");
				if (cookie_equal_sign == std::string::npos)
					continue;
				std::string cookie_key = val.substr(0, cookie_equal_sign);
				r._cookies[cookie_key] = val;
			}
			else
				r._headers[key] = val;
		}
	}

	r._status_text = getStatusText(r._status_code);
	r._headers["Content-Length"] = std::to_string(body.length());
	r._body = body;
	return (r);
}

void Dispatcher::checkForCgi(const HttpRequest& request, std::string& interpreter, std::string& path, std::vector<std::string>& env, LocationConfig* lc)
{
	std::string extension;

	if (request._path.find(".") != std::string::npos)
	{
		size_t	pos_dot = request._path.find_last_of('.');
		size_t	pos_slash_after_dot = request._path.substr(pos_dot).find('/');

		if (pos_slash_after_dot == std::string::npos) // no path info
		{
			extension = request._path.substr(pos_dot);
			path = request._path;
		}
		else // path info given
		{
			extension = request._path.substr(pos_dot, pos_slash_after_dot);
			env.push_back("PATH_INFO=" + request._path.substr(pos_dot + pos_slash_after_dot));
			path = request._path.substr(0, pos_dot + pos_slash_after_dot);
		}

		auto it = lc->_cgi_map.find(extension);
		if (it == lc->_cgi_map.end())
		{
			std::cout << "[INFO]  CGI extension not in cgi map, handle as static page" << std::endl;
			throw std::runtime_error("handle as static file");
		}
		interpreter = it->second;
	}
	else // no file given
	{
		std::cout << "[INFO]  CGI no file given, handle as static page" << std::endl;
		throw std::runtime_error("handle as static file");
	}
}

CgiSession Dispatcher::startCgi(const HttpRequest& request, ServerConfig* sc, LocationConfig* lc)
{
	CgiSession cs;

	std::string					interpreter;
	std::string					path;
	std::vector<std::string>	env;
	
	checkForCgi(request, interpreter, path, env, lc);

	// build path
	std::string script_path = getFullRootPath(lc) + path;
	this->isWithin(getFullRootPath(lc) + lc->_name, script_path);

	// file exist and readable?
	if (access(script_path.c_str(), F_OK) == -1)
		throw HttpException(404);
	if (access(script_path.c_str(), R_OK) == -1)
		throw HttpException(403);

	buildEnv(env, request, path, script_path, sc);

	// execve needs char* array
	std::vector<char*> envp;
	for (auto& s : env)
	{
		envp.push_back(s.data());
	}
	envp.push_back(nullptr);

	// create pipes and start cgi process
	int in_pipe[2];
	int out_pipe[2];
	if (pipe(in_pipe) == -1)
		throw HttpException(500);
	if (pipe(out_pipe) == -1)
	{
		close(in_pipe[0]);
		close(in_pipe[1]);
		throw HttpException(500);
	}
	pid_t pid = fork();
	if (pid == -1)
	{
		close(in_pipe[0]);
		close(in_pipe[1]);
		close(out_pipe[0]);
		close(out_pipe[1]);
		throw HttpException(500);
	}
	if (pid == 0) // child
	{
		dup2(in_pipe[0], STDIN_FILENO);
		dup2(out_pipe[1], STDOUT_FILENO);
		close(in_pipe[0]);
		close(in_pipe[1]);
		close(out_pipe[0]);
		close(out_pipe[1]);

		if (chdir(getFullRootPath(lc).c_str()) != 0) {
    		throw std::runtime_error("chdir failed");
		}
		char *argv[] = {interpreter.data(), script_path.data(), nullptr};
		execve(interpreter.c_str(), argv, envp.data());
		_exit(1);
	}

	std::cout << "[INFO]  CGI started pid " << pid << std::endl;
	cs._pid = pid;
	cs._stdin_fd = in_pipe[1];
	cs._stdout_fd = out_pipe[0];
	// std::cout << "[DEBUG] CGI pipe stdin_fd " << cs._stdin_fd << std::endl;
	// std::cout << "[DEBUG] CGI pipe stdout_fd " << cs._stdout_fd << std::endl;
	cs._started = time(nullptr);
	close(in_pipe[0]);
	close(out_pipe[1]);

	// wenn kein body vorhanden dann gleich schliessen
	if (request._body.empty())
	{
		close(in_pipe[1]);
		cs._stdin_fd = -1;
	}
	else
	{
		cs._body = request._body;
		fcntl(in_pipe[1], F_SETFL, O_NONBLOCK);
	}
	
	fcntl(out_pipe[0], F_SETFL, O_NONBLOCK);

	return (cs);
}

CgiSession Dispatcher::handleCgi(const HttpRequest& request, ServerConfig* sc, LocationConfig* lc)
{
	std::cout << "[INFO]  entered cgi handler" << std::endl;

	CgiSession cs = startCgi(request, sc, lc);
	return (cs);
}
