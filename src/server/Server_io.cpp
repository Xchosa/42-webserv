#include "Server.hpp"

void Server::acceptClient(int server_fd)
{
	while(true)
	{
		sockaddr_in client_addr;
  		socklen_t len = sizeof(client_addr);
		int client_fd = accept(server_fd, reinterpret_cast<sockaddr*>(&client_addr), &len);
		if(client_fd == -1)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
			break;
			throw std::runtime_error("accept failed");
		}

		setNonBlocking(client_fd);
		
		ClientInfos client;
		client._listen_context = &_socket_fds[server_fd];
		client._selected_server = NULL;
		client._last_activity = time(NULL);
		_clients[client_fd] = client;

		addFdEpoll(client_fd, EPOLLIN | EPOLLRDHUP); 
	}

}

int	Server::checklastActivity(int client_fd)
{
	int time_delta = time(NULL)- _clients[client_fd]._last_activity;
	return time_delta;
}

void Server::recvClientData(int client_fd)
{
	char buffer[4096];

	while (true)
	{
		ssize_t bytes = recv(client_fd, &buffer[0], sizeof(buffer), 0);
		if (bytes > 0)
		{
			_clients[client_fd]._last_activity = time(NULL);

			_clients[client_fd]._parser.feedBuffer(buffer, bytes);
			ParseStatus parse_status = _clients[client_fd]._parser.parseBuffer();

			if (parse_status == HEADER_COMPLETE) // header complete, but body missing
			{
				_clients[client_fd].selectVirtualHost();
				parse_status = _clients[client_fd]._parser.parseBuffer();
			}
			if (parse_status == COMPLETE && _clients[client_fd]._selected_server == nullptr) // request complete parsed, no body, search now for correct sever
				_clients[client_fd].selectVirtualHost();

			if (parse_status == COMPLETE)
			{
				std::cout << "[INFO]  Client " << client_fd << ": request complete" << std::endl;

				HttpResponse	response;
				CgiSession		cgi;
				DispatchResult	dp_result = _dispatcher.dispatch(_clients[client_fd]._parser.getRequest(), _clients[client_fd]._selected_server, response, cgi);
				if (dp_result == DP_DONE)
				{
					_clients[client_fd]._response = response;
				}
				if (dp_result == DP_CGI_PENIDNG)
				{
					_clients[client_fd]._cgi = cgi;

					addFdEpoll(cgi._stdout_fd, EPOLLIN);
					_cgi_fd_client_owner[cgi._stdout_fd] = client_fd;

					if (!cgi._body.empty()) // no body, no need to register stdin pipe
					{
						addFdEpoll(cgi._stdin_fd, EPOLLOUT);
						_cgi_fd_client_owner[cgi._stdin_fd] = client_fd;
					}
				}
				break;
			}
			else if (parse_status == ERROR_400)
			{
				_clients[client_fd]._response = _dispatcher.buildErrorResponse(400, _clients[client_fd]._selected_server, CON_CLOSE, _clients[client_fd]._parser.getRequest());
				break;
			}
			else if (parse_status == ERROR_413)
			{
				_clients[client_fd]._response = _dispatcher.buildErrorResponse(413, _clients[client_fd]._selected_server, CON_CLOSE, _clients[client_fd]._parser.getRequest());
				break;
			}

			if (checklastActivity(client_fd) >= KEEP_ALIVE_TIMEOUT)
			{
				closeClient(client_fd);
				break;
			}
		}
		else if (bytes == 0)
  		{
  			closeClient(client_fd);
  			break;
  		}
		else
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK) // no data available right now
  				break;
			closeClient(client_fd);
  			break;
		}
	}
}

void Server::sendToClient(int client_fd)
{
	ClientInfos& client = _clients[client_fd];

	if (client._response_buffer.empty())
			client._response_buffer = _clients[client_fd]._response.serialize();
	//std::string response = _clients[client_fd]._response.serialize();
	while (!client._response_buffer.empty())
	{
		ssize_t bytes = send(client_fd, client._response_buffer.c_str(), client._response_buffer.length(), 0 );
		if (bytes < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				  return;
			closeClient(client_fd);
			return;
		}
		if(bytes == 0)
			return;

		client._response_buffer.erase(0, bytes);
	}

	if (_clients[client_fd]._response._headers.count("Connection") && _clients[client_fd]._response._headers["Connection"] == "close")
	{
		closeClient(client_fd);
		return;
	}

	client._response = HttpResponse();
	client._response_buffer.clear();
}
