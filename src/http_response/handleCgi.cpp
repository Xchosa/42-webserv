#include "Dispatcher.hpp"

std::string Dispatcher::upperString(std::string str) const
{
	for (char &c : str)
	{
		if (c == '-')
			c = '_';
		else
			c = std::toupper(c);
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
	if (sc->_listen_host.length() > 0)
		env.push_back("SERVER_NAME=" + sc->_listen_host);
	if (request._query.length() > 0)
		env.push_back("QUERY_STRING=" + request._query);
	if (request._body.length() > 0)
		env.push_back("CONTENT-LENGTH=" + std::to_string(request._body.length()));
	if (upperString(path.substr(path.find_last_of('.'))) == ".PHP")
		env.push_back("REDIRECT_STATUS=200");
	if (request._headers.find("content-type") != request._headers.end())
		env.push_back("CONTENT_TYPE=" + request._headers.at("content-type"));

	// add request header
	for (auto &h : request._headers)
	{
		env.push_back("HTTP_" + upperString(h.first) + "=" + h.second);
	}
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
			std::cout << "extension not found in cgi map\n";
			throw HttpException(404); // TODO: als static file handeln?
		}
		interpreter = it->second;
	}
	else // no file given
	{
		std::cout << "no file given\n";
		throw HttpException(404); // TODO: als static file handeln?
	}
}

// HttpResponse Dispatcher::parseCgiOutput(std::string& cgi_output)
// {

// }

HttpResponse Dispatcher::handleCgi(const HttpRequest& request, ServerConfig* sc, LocationConfig* lc)
{
	std::cout << "> CGI HANDLER\n";

	std::string					interpreter;
	std::string					path;
	std::vector<std::string>	env;
	
	checkForCgi(request, interpreter, path, env, lc);

	// build path
	std::string script_path = getFullRootPath(lc) + path;
	// file exist and readable?
	if (access(script_path.c_str(), F_OK) == -1)
		throw HttpException(404);
	if (access(script_path.c_str(), R_OK) == -1)
		throw HttpException(403);

	buildEnv(env, request, path, script_path, sc);
	std::cout << "env's:\n";
	for (auto &s : env)
	{
		std::cout << "- " << s << std::endl;
	}

	// for execve char* array
	std::vector<char*> envp;
	for (auto& s : env)
	{
		envp.push_back(s.data());
	}
	envp.push_back(nullptr);


	int in_pipe[2];
	int out_pipe[2];
	pid_t pid;

	try
	{
		pipe(in_pipe);
		pipe(out_pipe);
		pid = fork();
	}
	catch(const std::exception& e)
	{
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

		chdir(getFullRootPath(lc).c_str());
		char *argv[] = {interpreter.data(), script_path.data(), nullptr};
		execve(interpreter.c_str(), argv, envp.data());
		std::exit(1);
	}
	
	// parent:
	// ########## dummy pumping data from parent to child ##########
	close(in_pipe[0]);
	close(out_pipe[1]);

	// body schreiben, danach weite schliessen fuer EOF
	write(in_pipe[1], request._body.data(), request._body.length());
	close(in_pipe[1]);

	// output lesen bis eof
	std::string cgi_output;
	char buf[1024];
	ssize_t n;
	while ((n = read(out_pipe[0], buf, sizeof(buf))) > 0)
		cgi_output.append(buf, n);
	close(out_pipe[0]);

	int status;
	waitpid(pid, &status, 0);
	// ########## END dummy pumping data from parent to child ##########


	std::cout << "cgi_out:\n" << cgi_output << std::endl;

	// HttpResponse r = parseCgiOutput(cgi_output);




	HttpResponse dummy;
	return (dummy);
}
