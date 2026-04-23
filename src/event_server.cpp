#include "inc/webserv.hpp"



#define PORT 8081
int main()
{
	int server_fd, new_socket;
	long valread;
	struct sockaddr_in address;
	int addrlen = sizeof(address);

	const char *MessageServer = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\r\n\r\nhello Server";
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

	while(1)
	{
		int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);

	}
}
//  keep list of all fds -> ask os which fd is ready 
// react only to the ready fds 
// go back and wait again