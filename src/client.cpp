#include "client.hpp"



//int main()
//{
//	int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
//	sockaddr_in serverAddress;
//	serverAddress.sin_family = AF_INET;
//	serverAddress.sin_port = htons(8080);
//	//serverAddress.sin_addr.s_addr = INADDR_ANY;
//	inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr);
//	//connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

//	const char* message = "Hello, server!";
//	send(clientSocket, message, strlen(message), 0);
//	close(clientSocket);

//}

#define PORT 8081

int main(int argc, char const *argv[])
{
    int sock = 0; long valread;
    struct sockaddr_in serv_addr;
    const char *hello = "Hello from client";
    char buffer[1024] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form -> adress strucure 
	// presentation to network 
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) 
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }
    send(sock , hello , strlen(hello) , 0 );
    printf("Hello message sent\n");
    valread = read( sock , buffer, 1024);
    printf("%s\n",buffer );
    close(sock);
    return 0;
}