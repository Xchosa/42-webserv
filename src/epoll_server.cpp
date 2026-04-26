#include "../inc/webserv.hpp"



#define PORT 8081

int main()
{
	int server_fd;
	struct sockaddr_in address;
	std::string MessageServer =
					"HTTP/1.1 200 OK\r\n"
					"Content-Type: text/html\r\n"
					"Content-Length: 13\r\n"
					"\r\n"
					"Hello, World!";

	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("In socket");
		exit(EXIT_FAILURE);
	}
}
                
