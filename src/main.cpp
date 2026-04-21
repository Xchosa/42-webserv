#include "client.hpp"


int main()
{
	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	// get fd for Server 

//AF_INET : IPv4 protocol
//SOCK_STREAM: TCP socket
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(8080);
	serverAddress.sin_addr.s_addr = INADDR_ANY;

	//htons(): Converts port to network byte order.
	//INADDR_ANY: Accept connections on any IP.

	bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
	// Bind soccet to address
	listen(serverSocket, 5);

	// fcntl(serverSocket, flag ) // non blocking umstellen 

	int clientSocket = accept(serverSocket, nullptr, nullptr);

	char buffer[1024] = {0};
	recv(clientSocket, buffer, sizeof(buffer), 0);
	std::cout << "Message from client: " << buffer << std::endl;
	
	close(serverSocket);

	return 0;
}