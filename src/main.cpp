#include "webserv.hpp"
#include "Lexer.hpp"

#define PORT 8081
#define MAX_EVENTS 10

void printTokens(const std::vector<Token>& tokens)
{
    for (const Token& t : tokens)
    {
        std::string type;
        switch (t.type)
        {
            case WORD:        type = "WORD       "; break;
            case SEMICOLIN:   type = "SEMICOLIN  "; break;
            case LBRACE:      type = "LBRACE     "; break;
            case RBRACE:      type = "RBRACE     "; break;
            case END_OF_FILE: type = "EOF        "; break;
        }
        std::cout << "[line " << std::setw(3) << t.line << "] "
                  << type << " | " << t.value << "\n";
    }
}

Config parseConfig(std::string conf_file_path)
{
	Lexer l(conf_file_path);
	std::vector<Token> tokens = l.buildTokens();
	if (tokens.size() == 0)
		throw std::runtime_error("Empty config file!");



	printTokens(tokens);

	Config config;
	return (config);
}

int main()
{
	// begin parser
	try
	{
		Config config;
		// config = parseConfig("conf/example_1.conf");
		// config = parseConfig("conf/example_2_big.conf");
		config = parseConfig("conf/example_3_empty.conf");


	}
	catch(const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << '\n';
	}
	return (0);
	// end parser

	int server_fd, epoll_fd;
	struct sockaddr_in address;
	struct epoll_event event, events[MAX_EVENTS];

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

	// Set socket to non-blocking
	int flags = fcntl(server_fd, F_GETFL, 0);
	if (flags == -1)
	{
		perror("fcntl F_GETFL");
		exit(EXIT_FAILURE);
	}
	if (fcntl(server_fd, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		perror("fcntl F_SETFL O_NONBLOCK");
		exit(EXIT_FAILURE);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		perror("In bind");
		exit(EXIT_FAILURE);
	}

	if (listen(server_fd, SOMAXCONN) < 0)
	{
		perror("In listen");
		exit(EXIT_FAILURE);
	}

	if ((epoll_fd = epoll_create1(0)) < 0)
	{
		perror("epoll_create1");
		exit(EXIT_FAILURE);
	}

	event.events = EPOLLIN;
	event.data.fd = server_fd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) < 0)
	{
		perror("epoll_ctl");
		exit(EXIT_FAILURE);
	}

	while (1)
	{
		int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
		if (n < 0)
		{
			perror("epoll_wait");
			continue;
		}

		for (int i = 0; i < n; i++)
		{
			if (events[i].data.fd == server_fd)
			{
				// New connection
				struct sockaddr_in client_addr;
				socklen_t client_len = sizeof(client_addr);
				int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
				if (client_fd < 0)
				{
					perror("accept");
					continue;
				}

				// Set client socket to non-blocking
				flags = fcntl(client_fd, F_GETFL, 0);
				fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

				event.events = EPOLLIN | EPOLLET; // Edge-triggered for new data
				event.data.fd = client_fd;
				if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) < 0)
				{
					perror("epoll_ctl add client");
					close(client_fd);
				}
			}
			else
			{
				// Data from a client
				int client_fd = events[i].data.fd;
				std::vector<char> buffer(1024);
				int bytes_read = read(client_fd, buffer.data(), buffer.size());

				if (bytes_read <= 0)
				{
					// Connection closed or error
					epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL); // deletes epoll instance 
					close(client_fd);
				}
				else
				{
					// We got a request, send a response
					send(client_fd, MessageServer.c_str(), MessageServer.length(), 0);
					// In a real server, you'd parse the request first.
					// For this simple case, we just close the connection after sending.
					epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
					close(client_fd);
				}
			}
		}
	}

	close(server_fd);
	return 0;
}
// telnet localhost 8081
// GET / HTTP/1.0