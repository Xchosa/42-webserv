#include "client.hpp"

// g++ -std=c++17 server.cpp -o server

//int main()
//{
//	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
//	//
//	// get fd for Server

//	// AF_INET : IPv4 protocol
//	// SOCK_STREAM: TCP socket
//	sockaddr_in serverAddress;
//	serverAddress.sin_family = AF_INET;
//	serverAddress.sin_port = htons(8080);
//	serverAddress.sin_addr.s_addr = INADDR_ANY;

//	// htons(): Converts port to network byte order.
//	// INADDR_ANY: Accept connections on any IP.

//	bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
//	// Bind soccet to address
//	listen(serverSocket, 5);

//	// fcntl(serverSocket, flag ) // non blocking umstellen

//	int clientSocket = accept(serverSocket, nullptr, nullptr);

//	char buffer[1024] = {0};
//	recv(clientSocket, buffer, sizeof(buffer), 0);
//	std::cout << "Message from client: " << buffer << std::endl;

//	close(serverSocket);

//	return 0;
//}

#define PORT 8081
int main()
{
	int server_fd, new_socket;
	long valread;
	struct sockaddr_in address;
	int addrlen = sizeof(address);

	const char *MessageServer = "Hello from server";

	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("In socket");
		exit(EXIT_FAILURE);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT); // network short number


	//memset(address.sin_zero, '\0', sizeof address.sin_zero); without mention it is initialized to zero 

	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		perror("In bind");
		exit(EXIT_FAILURE);
	}
	if (listen(server_fd, 10) < 0) // max 10 connections 
	{
		perror("In listen");
		exit(EXIT_FAILURE);
	}
	while (1)
	{
		printf("\n+++++++ Waiting for new connection ++++++++\n\n");
		if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
		{
			perror("In accept");
			exit(EXIT_FAILURE);
		}

		char buffer[30000] = {0};
		valread = read(new_socket, buffer, 30000);
		printf("%s\n", buffer);
		write(new_socket, MessageServer, strlen(MessageServer));
		printf("------------------Hello message sent-------------------\n");
		close(new_socket);
	}
	return 0;
}