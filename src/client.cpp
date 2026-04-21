#include "client.hpp"



int main()
{
	int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(8080);
	//serverAddress.sin_addr.s_addr = INADDR_ANY;
	inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr);
	connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

	const char* message = "Hello, server!";
	send(clientSocket, message, strlen(message), 0);
	close(clientSocket);

}