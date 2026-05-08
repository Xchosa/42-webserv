
#include "webserv.hpp"




void Server::acceptFd(int server_fd)
{
	// accept server_socket with client fd 
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

	// add client_fd to epoll
	}

}


void Server::readFd(int client_fd)
{
	// recv()
	// read()
	std::string buffer;
	buffer.resize(4096);
	while(true)
	{
		ssize_t bytes = recv(client_fd, &buffer[0], sizeof(buffer),0  );

		if(bytes >0)
		{
			//ParseStatus tmp_status = _clients[client_fd]._parser.feed()
		}
	}
}

void Server::writeFD(int client_fd)
{
	void();
}
