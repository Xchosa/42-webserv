#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <vector>

int main(void)
{
	// creating TCP socket
	int server_fd = socket(PF_INET, SOCK_STREAM, 0);
	if (server_fd == -1)
	{
		perror("Error socket");
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
	sockaddr_in sin;
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


	// init for poll
	std::vector<pollfd> fds;
	pollfd server_pollfd;
	server_pollfd.fd = server_fd;
	server_pollfd.events = POLLIN;
	fds.push_back(server_pollfd);

	while (true)
	{
		int ready = poll(fds.data(), fds.size(), -1);
		if (ready == -1)
		{
			perror("Error poll");
			return (1);
		}

		// durch alle fds in pool loopen und schauen wer ready ist
		for (size_t i = 0; i < fds.size(); i++)
		{
			// server_fd ist ready -> neuer client will connecten
			if (fds[i].fd == server_fd && fds[i].revents & POLLIN)
			{
				// original socket is only used for accepting connections, not for exchanging data
				sockaddr_storage client;
				socklen_t client_len = sizeof(client);
				int client_fd = accept(server_fd, (sockaddr *)&client, &client_len);
				if (client_fd == -1)
				{
					std::cerr << "Error accept\n";
					return (1);
				}
				// add new client to fds
				pollfd client_pollfd;
				client_pollfd.fd = client_fd;
				client_pollfd.events = POLLIN;
				fds.push_back(client_pollfd);
			}
			// client_fd ist ready -> daten lesen
			else if (fds[i].revents & POLLIN)
			{
				char buffer[10000] = {0};
				int valread = recv(fds[i].fd, buffer, sizeof(buffer), 0);
				if (valread <= 0)
				{
					std::cout << "Error recv\n";
					close(fds[i].fd);
					fds.erase(fds.begin() + i);
					i--;
					continue;
				}
				printf("%s\n", buffer);
				std::string response =
					"HTTP/1.1 200 OK\r\n"
					"Content-Type: text/html\r\n"
					"Content-Length: 13\r\n"
					"\r\n"
					"Hello, World!";
				
				send(fds[i].fd, response.c_str(), response.length(), 0);
				close(fds[i].fd);
				fds.erase(fds.begin() + i);
				i--;
			}
			// fehler, verbindung getrennt
			else if (fds[i].revents & (POLLERR | POLLHUP))
			{
				close(fds[i].fd);
				fds.erase(fds.begin() + i);
				i--;
			}
		}
	}
}



