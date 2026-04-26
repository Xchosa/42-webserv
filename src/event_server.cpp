#include "../inc/webserv.hpp"



#define PORT 8081

int main()
{
	int server_fd;
	struct sockaddr_in address;


	//const char *MessageServer = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 12\r\n\r\nhello Server";
	std::string MessageServer =
					"HTTP/1.1 200 OK\r\n"
					"Content-Type: text/html\r\n"
					"Content-Length: 13\r\n"
					"\r\n"
					"Hello, World!";
                


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


	//struct pollfd fds[MAXCLIENT + 1];
	// poll array 
	std::vector<pollfd> fds;
	pollfd server_pollfd;		// container of servers 
	server_pollfd.fd = server_fd; // 
	server_pollfd.events = POLLIN; // flag there is data to read 
	fds.push_back(server_pollfd);

	
	
	while(1)
	{
		//int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
		//event_server_fd =  poll(struct pollfd *fds, nfds_t nfds, int timeout);
		
		// 
		int ready = poll(fds.data(), fds.size(), -1); // block until listening socket becomes ready 
		if (ready < 0 )
		{
            perror("poll");
            break;
        }
		for (size_t i = 0 ; i < fds.size() ; i++)
		{
			if(fds[i].fd == server_fd && fds[i].revents & POLLIN) 
			// connection is established ++ pollin flag happend 
			// retruns connecting fd
			{
				sockaddr_storage client; // // hold clients ip adress + Port ifno
				socklen_t client_len = sizeof(client); // store byte size 
				
				// client ownwership 
				int client_fd = accept(server_fd, reinterpret_cast<sockaddr *>(&client), &client_len); // 
				// which client =  which socket is recieving infos , cast to sockaddr pointer 
				if (client_fd < 0 )
				{
					perror("accept");
					continue;
				}
				pollfd client_pollfd;
				client_pollfd.fd = client_fd;
				client_pollfd.events = POLLIN;
				fds.push_back(client_pollfd);
			}
			else if(fds[i].revents & POLLIN)
			// correct client id with pollin Flag
            {
				std::vector<char> buffer(5000);
				int valread = recv(fds[i].fd, buffer.data(), buffer.size(), 0);

                if (valread <= 0)
                {
                    // client closed connection or error
					std::cout << "Error recv\n";
                    close(fds[i].fd);
                    fds.erase(fds.begin()+i);
                    i--; // re-check the moved entry
                    continue;
                }
				send(fds[i].fd, MessageServer.c_str(), MessageServer.length(), 0);
				//send(fds[i].fd, buffer.data(), buffer.size(),0 );
				close(fds[i].fd);
				fds.erase(fds.begin()+ i);
				i--;
			}
			else if (fds[i].revents & (POLLERR | POLLHUP))
			{
				close(fds[i].fd);
				fds.erase(fds.begin()+ i);
				i--;
			}
        }
    }

}
//  keep list of all fds -> ask os which fd is ready 
// react only to the ready fds 
// go back and wait again