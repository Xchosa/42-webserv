#include "Server.hpp"

void Server::cgiWriteBody(int pipe_fd, int client_fd, CgiSession* cgi)
{
	while (!cgi->_body.empty())
	{
		ssize_t n = write(cgi->_stdin_fd, cgi->_body.data(), cgi->_body.length());
		if (n == -1)
		{
			if (errno == EAGAIN)
				break;
			killCgi(client_fd, 502);
			return ;
		}
		else if (n > 0)
		{
			cgi->_body.erase(0, n);
		}
	}
	if (cgi->_body.empty())
	{
		removeFdEpoll(cgi->_stdin_fd);
		close(cgi->_stdin_fd);
		cgi->_stdin_fd = -1;
		_cgi_fd_client_owner.erase(pipe_fd);
	}
}

void Server::cgiReadOutput(int pipe_fd, int client_fd, ClientInfos* client, CgiSession* cgi)
{
	char	buffer[1024];
	ssize_t	n;
	while (true)
	{
		n = read(pipe_fd, buffer, sizeof(buffer));
		if (n > 0)
			cgi->_output.append(buffer, n);
		else if (n == 0)
		{
			std::cout << "[INFO]  CGI output successfully readed" << std::endl;
			removeFdEpoll(pipe_fd);
			close(cgi->_stdout_fd);
			cgi->_stdout_fd = -1;
			_cgi_fd_client_owner.erase(pipe_fd);

			int status;
			pid_t result = waitpid(cgi->_pid, &status, WNOHANG);
			if (result == cgi->_pid) // child successfully ended
			{
				cgi->_waited = true;
				if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
				{
					std::cout << "[INFO]  CGI pid " << result << " ended with error" << std::endl;
					client->_response = _dispatcher.buildErrorResponse(502, client->_selected_server, CON_KEEP_ALIVE, client->_parser.getRequest());
				}
				else
				{
					try
					{
						client->_response = _dispatcher.parseCgiOutput(cgi->_output);
						std::cout << "[INFO]  CGI pid " << result << " ended good" << std::endl;
					}
					catch(const HttpException& e)
					{
						std::cout << "[INFO]  CGI pid " << result << " ended good but invalid cgi response" << std::endl;
						client->_response = _dispatcher.buildErrorResponse(e.code(), client->_selected_server, CON_KEEP_ALIVE, client->_parser.getRequest());
					}
				}
			}
			else if (result == 0)
			{
				std::cout << "[INFO]  CGI pid " << cgi->_pid << " still running" << std::endl;
			}
			else
			{
				std::cout << "[INFO]  CGI pid " << cgi->_pid << " error on waitpid" << std::endl;
				client->_response = _dispatcher.buildErrorResponse(502, client->_selected_server, CON_KEEP_ALIVE, client->_parser.getRequest());
				cgi->_waited = true;
			}
			break;
		}
		else if (n == -1)
		{
			if (errno == EAGAIN)
			{
				break;
			}
			killCgi(client_fd, 502);
			return ;
		}
	}
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
	else if (pipe_fd == cgi->_stdin_fd && (event_flag & (EPOLLOUT | EPOLLHUP)))
	{
		if (event_flag & EPOLLHUP)
		{
			std::cout << "[INFO]  CGI writing done, close stdin" << std::endl;
			removeFdEpoll(pipe_fd);
			close(pipe_fd);
			cgi->_stdin_fd = -1;
			_cgi_fd_client_owner.erase(pipe_fd);
			return ;
		}
		std::cout << "[INFO]  CGI write body to cgi stdin" << std::endl;
		cgiWriteBody(pipe_fd, client_fd, cgi);
	}
	else if (pipe_fd == cgi->_stdout_fd && (event_flag & (EPOLLIN | EPOLLHUP)))
	{
		std::cout << "[INFO]  CGI read output" << std::endl;
		cgiReadOutput(pipe_fd, client_fd, client, cgi);
	}
}
