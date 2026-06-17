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

HttpResponse DUMMY_response_OK()
{
	HttpResponse re;

	re._status_code = 200;
	re._status_text = "OK";
	re._version = "HTTP/1.1";
	re._body = "Okay!\n";
	re._headers["Content-Length"] = std::to_string(re._body.length());
	re._headers["Content-Type"] = "text/plain";
	re._headers["Connection"] = "keep-alive";

	return (re);
}

HttpResponse DUMMY_response_ERR400()
{
	HttpResponse re;

	re._status_code = 400;
	re._status_text = "Bad Request";
	re._version = "HTTP/1.1";
	re._body = "Fehler du versager!\n";
	re._headers["Content-Length"] = std::to_string(re._body.length());
	re._headers["Content-Type"] = "text/plain";
	re._headers["Connection"] = "close";

	return (re);
}

int	Server::checklastActivity(int client_fd)
{
	int time_delta = time(NULL)- _clients[client_fd]._last_activity;
	return time_delta;
}

void Server::recvClientData(int client_fd)
{
	char buffer[4096];
	Dispatcher dpatch;

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
				_clients[client_fd]._parser.printRequest();
				std::cout << "Request complete from client_fd: " << client_fd << std::endl;
				
				
				// 2. dispatcher aufrufen um passende location rauszusuchen und handler aufzurufen
				_clients[client_fd]._response = dpatch.dispatch(_clients[client_fd]._parser.getRequest(), _clients[client_fd]._selected_server);
				// _clients[client_fd]._response = DUMMY_response_OK();


				//modifyFdEpoll(client_fd, EPOLLOUT | EPOLLRDHUP);
				break;
			}
			else if (parse_status == ERROR_400)
			{
				// error reponse muss noch raus an den client, nicht direkt schliessen
				// _clients[client_fd]._response = DUMMY_response_ERR400();
				_clients[client_fd]._response = dpatch.buildErrorResponse(400, _clients[client_fd]._selected_server, CON_CLOSE);
				//modifyFdEpoll(client_fd, EPOLLOUT | EPOLLRDHUP);
				// closeClient(client_fd);
				break;
			}
			else if (parse_status == ERROR_413)
			{
				_clients[client_fd]._response = dpatch.buildErrorResponse(413, _clients[client_fd]._selected_server, CON_CLOSE);
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
			if (errno == EAGAIN || errno == EWOULDBLOCK)// no data available right now
  				break;
			closeClient(client_fd);
  			break;
		}
	}
}

void Server::sendToClient(int client_fd)
{

	std::string response = _clients[client_fd]._response.serialize();
	while (!response.empty())
	{
		int i = 0;
		i++;
		ssize_t bytes = send(client_fd, response.c_str(), response.length(), 0 );
		if (bytes < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				  return;
			closeClient(client_fd);
			return;
		}
		response.erase(0, bytes);

	}
	// std::cout << "Completed data sending to Browser from client_fd: " << client_fd << std::endl;

	if (_clients[client_fd]._response._headers.count("Connection") && _clients[client_fd]._response._headers["Connection"] == "close")
	{
		closeClient(client_fd);
	}
}
