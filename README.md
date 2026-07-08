*This project has been created as part of the 42 curriculum by ghambrec and poverbec.*

# Webserv

## Description

`webserv` is a small HTTP/1.1 web server written in C++17. The goal of the project is to understand how a browser and a web server communicate over TCP, how HTTP requests are parsed, and how responses are generated.

The server reads a configuration file, opens one or more listening sockets, accepts clients with non-blocking I/O and `epoll`, parses HTTP requests, selects the matching virtual host and location, then dispatches the request to the correct handler.

Implemented features include:

- `GET`, `POST`, and `DELETE`
- static file serving
- configurable server blocks and locations
- virtual hosts through the `Host` header
- file upload
- file deletion
- redirects
- autoindex
- custom error pages
- CGI execution by file extension, for example Python or PHP
- `Content-Length` and chunked request bodies
- keep-alive connections

## Instructions

### Build

```bash
make
```

The executable is created as:

```bash
./webserv
```

Useful Makefile targets:

```bash
make          # optimized build
make clean    # remove object files
make fclean   # remove object files and executable
make re       # rebuild from scratch
```

### Run

Run the server with a config file:

```bash
./webserv conf/gha.conf
```

or:

```bash
./webserv conf/pov.conf
```

The program expects exactly one config file argument:

```bash
./webserv <config_file>
```

### Test

Use a browser:

```text
http://localhost:8081/
```

Use `nc` for raw HTTP requests:

```bash
printf 'GET / HTTP/1.1\r\nHost: vogel\r\nConnection: close\r\n\r\n' | nc localhost 8081
```

Use `curl`:

```bash
curl -i http://localhost:8081/
```

Run the included request scripts:

```bash
./TestRequests/testPostDeleteGet.sh
./TestRequests/testparallelCGI.sh
```

Use `siege` for load testing:

```bash
siege -b -c50 -t30S http://127.0.0.1:8081/

or use the TestRequest/siegeTest.sh script to stress test multiple servers 
```

### Common Issues

If a port is already in use:

```bash
lsof -i :8081
kill -9 <PID>
```

To inspect listening TCP sockets:

```bash
ss -tlpn
```

## Project Structure

```text
inc/                  Header files
src/                  Source files
src/config_parser/    Config lexer and parser
src/http_request/     HTTP request parser
src/http_response/    Dispatcher, handlers, responses, CGI
src/server/           Sockets, epoll loop, clients
conf/                 Example configuration files
TestRequests/         Manual test requests and scripts
docs/                 Additional explanations and component notes
bbqserv/              Example website and CGI files
```

## More Information

Detailed project notes are available in:

- `docs/ConfigParser.md` - config parser rules
- `docs/HttpRequest.md` - HTTP parser rules
- `docs/Dispatcher.md` - routing, handlers, CGI, and errors
- `docs/ConfigServer.md` - server config constants

The README is intentionally shorter than the full documentation. It gives a quick overview and points to the deeper docs instead of duplicating everything.

## Resources

Classic references used for this topic:

- RFC 9110 - HTTP Semantics: https://www.rfc-editor.org/rfc/rfc9110
- RFC 9112 - HTTP/1.1: https://www.rfc-editor.org/rfc/rfc9112
- CGI/1.1 specification: https://datatracker.ietf.org/doc/html/rfc3875
- Linux `socket(2)`: https://man7.org/linux/man-pages/man2/socket.2.html
- Linux `bind(2)`: https://man7.org/linux/man-pages/man2/bind.2.html
- Linux `listen(2)`: https://man7.org/linux/man-pages/man2/listen.2.html
- Linux `accept(2)`: https://man7.org/linux/man-pages/man2/accept.2.html
- Linux `recv(2)`: https://man7.org/linux/man-pages/man2/recv.2.html
- Linux `send(2)`: https://man7.org/linux/man-pages/man2/send.2.html
- Linux `epoll(7)`: https://man7.org/linux/man-pages/man7/epoll.7.html
- Linux `fork(2)`: https://man7.org/linux/man-pages/man2/fork.2.html
- Linux `execve(2)`: https://man7.org/linux/man-pages/man2/execve.2.html
- Linux `pipe(2)`: https://man7.org/linux/man-pages/man2/pipe.2.html
- NGINX documentation for behavior comparison: https://nginx.org/en/docs/
- Mozilla HTTP overview: https://developer.mozilla.org/en-US/docs/Web/HTTP/Overview

### AI Usage

AI assistance was used as a learning and documentation aid. It helped with:

- explaining HTTP, CGI, request parsing, and web server concepts
- drafting and refining documentation files
- creating simple manual test scripts for `nc`, `/dev/tcp`, and `siege`
- reviewing project structure and summarizing how components interact
- generating small example CGI pages used for learning

AI was not used as a replacement for understanding the mandatory project concepts. The implementation decisions, debugging, integration, and validation remain the responsibility of the project authors.
