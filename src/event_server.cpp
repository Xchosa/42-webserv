#include "../inc/webserv.hpp"



#define PORT 8081
#define MAXCLIENT 10

int main()
{
	int server_fd, new_socket;
	long valread;
	struct sockaddr_in address;
	int addrlen = sizeof(address);

	const char *MessageServer = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 12\r\n\r\nhello Server";
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


	struct pollfd fds[MAXCLIENT + 1];
	// poll array 
	int nfds = 1; // only care about one fd -> listening socket 
	fds[0].fd = server_fd; // array of servers 
	fds[0].events = POLLIN; // new connection waiting in accept queue  // array of events/ client connections 

	while(1)
	{
		//int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
		//event_server_fd =  poll(struct pollfd *fds, nfds_t nfds, int timeout);
		
		// 
		int ready = poll(fds, nfds, -1); // block until listening socket becomes ready 
		if (ready < 0 )
		{
            perror("poll");
            break;
        }
		for (int i = 0 ; i < nfds ; i++)
		{
			if(fds[i].fd == ready) // retruns connecting fd
			{
				int client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
				if (client_fd < 0 )
				{
					perror("accept");
					continue;
				}
				if (nfds < MAXCLIENT + 1)
				{
					fds[nfds].fd = client_fd;
					fds[nfds].events = POLLIN;
					nfds++;
					printf("new client connected: fd = %d\n", client_fd);
				}
				else 
					perror("to many client: nfds= %d\n, nfds")
			
			else
			{
				
			}
			}
		}
		
	}
}
//  keep list of all fds -> ask os which fd is ready 
// react only to the ready fds 
// go back and wait again