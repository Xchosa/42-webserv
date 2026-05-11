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
		client._listen_context = _socket_fds[server_fd];
		client._selected_server = NULL;
		_clients[client_fd] = client;

		addFdEpoll(client_fd, EPOLLIN | EPOLLRDHUP); 
	}

}


std::string buildHelloWorldResponse()
{
	std::string body = "Hello Gabriel on the 11th of May\n";

	return "HTTP/1.1 200 OK\r\n"
			"Content-Type: text/plain\r\n"
			"Content-Length: " + std::to_string(body.size()) + "\r\n"
			"Connection: close\r\n"
			"\r\n" +
			body;
}


void Server::recvClientData(int client_fd)
{
	//std::string buffer;
	//buffer.resize(4096);
	char buffer[4096];
	while(true)
	{
		ssize_t bytes = recv(client_fd, &buffer[0], sizeof(buffer),0  );

		if(bytes >0)
		{
			ParseStatus tmp_status = _clients[client_fd]._parser.feed(buffer, bytes);


			if(tmp_status == COMPLETE)
			{
				std::cout << "Request complete from client_fd: " << client_fd << std::endl;
				std::cout << _clients[client_fd]._parser.TMP_getRawBuffer() << std::endl;
				_clients[client_fd]._response_buffer = buildHelloWorldResponse();
				modifyFdEpoll(client_fd, EPOLLOUT | EPOLLRDHUP);
				break;
			}
			if(tmp_status == ERROR)
			{
				removeFdEpoll(client_fd);
				close(client_fd);
				_clients.erase(client_fd);
				break;
			}
		}
		else if (bytes == 0)
  		{
  			removeFdEpoll(client_fd);
  			close(client_fd);
  			_clients.erase(client_fd);
  			break;
  		}
		else
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)// no data available right now
  				break;
			removeFdEpoll(client_fd);
  			close(client_fd);
  			_clients.erase(client_fd);
  			break;

		}

	}
}

void Server::sendToClient(int client_fd)
{
	std::string response = _clients[client_fd]._response_buffer;

	ssize_t bytes = send(client_fd, response.c_str(), response.length(), 0 );
	std::cout << "Completed data sending to Browser from client_fd: " << client_fd << std::endl;
	if (bytes < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
  			return;
		removeFdEpoll(client_fd),
		close(client_fd);
		_clients.erase(client_fd);
		return;

	}
	response.erase(0, bytes);

  	if (response.empty())
  	{
  		removeFdEpoll(client_fd);
  		close(client_fd);
  		_clients.erase(client_fd);
  	}

}
