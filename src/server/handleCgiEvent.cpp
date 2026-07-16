#include "Server.hpp"

CgiWriteResult Server::cgiWriteBody( CgiSession& cgi)
{
	if(cgi._body.empty())
		return CgiWriteResult::COMPLETE;
	ssize_t n = write(cgi._stdin_fd, cgi._body.data(), cgi._body.length());
	if (n < 0)
	{
		return CgiWriteResult::FAILED;
	}
	if (n == 0)
		return CgiWriteResult::PENDING; // no bytes written, _body can still contain data
	
	cgi._body.erase(0, n);
	if(cgi._body.empty())	
		return CgiWriteResult::COMPLETE;

	return CgiWriteResult::PENDING;
}

void Server::closeCgiStdout(CgiSession& cgi)
{
	if (cgi._stdout_fd == -1)
          return; // makes cleanup idempotent
	removeFdEpoll(cgi._stdout_fd);
	close(cgi._stdout_fd);
	_cgi_fd_client_owner.erase(cgi._stdout_fd);
	cgi._stdout_fd = -1;
}

void Server::tryFinishCgi(ClientInfos& client, CgiSession& cgi)
{
	int status = 0;
	pid_t result = waitpid(cgi._pid, &status, WNOHANG);
	if (result == 0)
	{
		std::cout << "[INFO]  CGI pid " << cgi._pid << " still running" << std::endl;
	}
	if(result < 0)
	{
		std::cout << "[INFO]  CGI pid " << cgi._pid << " error on waitpid" << std::endl;
		client._response = _dispatcher.buildErrorResponse(502, client._selected_server, CON_KEEP_ALIVE, client._parser.getRequest());
		cgi._waited = true;
		return;
	}
	cgi._waited = true; // child successfully ended

	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
	{
		std::cout << "[INFO]  CGI pid " << result << " ended with error" << std::endl;
		client._response = _dispatcher.buildErrorResponse(502, client._selected_server, CON_KEEP_ALIVE, client._parser.getRequest());
	}
	else
	{
		try
		{
			client._response = _dispatcher.parseCgiOutput(cgi._output);
			std::cout << "[INFO]  CGI pid " << result << " ended good" << std::endl;
		}
		catch(const HttpException& e)
		{
			std::cout << "[INFO]  CGI pid " << result << " ended good but invalid cgi response" << std::endl;
			client._response = _dispatcher.buildErrorResponse(e.code(), client._selected_server, CON_KEEP_ALIVE, client->_parser.getRequest());
		}
	}
}

CgiReadResult Server::cgiReadOutput(CgiSession& cgi)
{
	char	buffer[1024];
	ssize_t	n;
	
	n = read(cgi._stdout_fd, buffer, sizeof(buffer));
	if (n < 0)
		return CgiReadResult::FAILED;
	else if (n == 0)
	{
		return CgiReadResult::ENDOFFILE;
	}
	cgi._output.append(buffer, n);
	return CgiReadResult::PENDING;
}

void Server::handleCgiEvent(int pipe_fd, uint32_t event_flag)
{
	int				client_fd = _cgi_fd_client_owner[pipe_fd];	// fd from current client
	ClientInfos*	client = &_clients[client_fd];				// current client
	CgiSession*		cgi;										// cgi session from the client

	if (client->_cgi.has_value())
		cgi = &client->_cgi.value();
	else
	{
		killCgi(client_fd, 502);
		return ;
	}

	// std::cout << "[DEBUG] called handleCgiEvent with fd=" << pipe_fd << " flags=" << event_flag << "\n";
	if (event_flag & EPOLLERR)
	{
		killCgi(client_fd, 502);
	}
	CgiWriteResult result = cgiWriteBody(&cgi);
	if(result == CgiWriteResult::COMPLETE)
	{
		removeFdEpoll(pipe_fd);
		_cgi_fd_client_owner.erase(pipe_fd);
		close(pipe_fd);
		cgi->_stdin_fd = -1;
	}
	else if (result == CgiWriteResult::FAILED)
	{
		killCgi(client_fd, 502);
		return;
	}
}

