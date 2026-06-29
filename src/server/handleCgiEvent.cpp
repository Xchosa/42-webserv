#include "Server.hpp"

void Server::handleCgiEvent(int pipe_fd, uint32_t event_flag)
{
	ClientInfos*	client = &_clients[_cgi_fd_client_owner[pipe_fd]];	// current client
	CgiSession*		cgi = &client->_cgi.value();						// cgi session from the client

	std::cout << "[DEBUG] called handleCgiEvent with fd=" << pipe_fd << " flags=" << event_flag << "\n";

	if (event_flag & EPOLLERR)
	{
		killCgi(pipe_fd);
	}
	else if (pipe_fd == cgi->_stdin_fd && (event_flag & EPOLLOUT)) // body schreiben
	{
		std::cout << "[INFO]  CGI write body to cgi stdin\n";

		while (!cgi->_body.empty())
		{
			ssize_t n = write(cgi->_stdin_fd, cgi->_body.data(), cgi->_body.length());
			if (n == -1)
			{
        		if (errno == EAGAIN)
		            break;
				killCgi(pipe_fd);
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
	else if (pipe_fd == cgi->_stdout_fd && (event_flag & (EPOLLIN | EPOLLHUP)))// _stdout_fd -> EPOLLIN -> cgi output lesen
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
				cgi->_stdout_eof = true;
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
						std::cout << "[INFO]  CGI pid " << result << " ended good" << std::endl;
						client->_response = _dispatcher.parseCgiOutput(cgi->_output);
					}
				}
				else if (result == 0)
				{
					std::cout << "[INFO]  CGI pid " << cgi->_pid << " still running" << std::endl;
				}
				else
				{
					std::cout << "[INFO]  CGI pid " << cgi->_pid << " ERROR ON WAITPID!!!!" << std::endl;
					client->_response = _dispatcher.buildErrorResponse(502, client->_selected_server, CON_KEEP_ALIVE, client->_parser.getRequest());
				}

				break;
			}
			else if (n == -1)
			{
				if (errno == EAGAIN)
				{
					break;
				}
				killCgi(pipe_fd);
			}
		}
	}
}
