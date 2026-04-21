#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>


int main(void)
{
	int server_fd;
	sockaddr_in sin;
	sockaddr_storage client;

	// creating TCP socket
	server_fd = socket(PF_INET, SOCK_STREAM, 0);
	if (server_fd == -1)
	{
		// ...
		return (1);
	}

	// REUSEADDR
	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		perror("Error setsockopt");
		return (1);
	}

	// name the socket -> assign a port
	sin.sin_family = AF_INET;
	sin.sin_port = htons(8081);
	sin.sin_addr.s_addr = htonl(INADDR_ANY); // define interface, 0.0.0.0 -> os can choose whatever it wants
	if (bind(server_fd, (sockaddr *)&sin, sizeof(sin)) == -1)
	{
		perror("Error bind");
		return (1);
	}


	// wait for incoming connection
	if (listen(server_fd, 1000) == -1)
	{
		std::cerr << "Error listen\n";
		return (1);
	}

	while (true)
	{
		// original socket is only used for accepting connections, not for exchanging data
		socklen_t client_len = sizeof(client);
		int socket_client = accept(server_fd, (sockaddr *)&client, &client_len);
		if (socket_client == -1)
		{
			std::cerr << "Error accept\n";
			return (1);
		}
		

		char buffer[10000] = {0};
		int valread = read( socket_client , buffer, 1024); 
		printf("%s\n",buffer );
		if(valread < 0)
		{ 
			printf("No bytes are there to read");
		}
		std::string response =
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: text/html\r\n"
			"Content-Length: 13\r\n"
			"\r\n"
			"Hello, World!";
		send(socket_client, response.c_str(), response.length(), 0);
		close(socket_client);
	}
}



