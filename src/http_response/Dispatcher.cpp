#include "Dispatcher.hpp"
#include "HttpException.hpp"
#include "HttpStatus.hpp"

#include <algorithm>
#include <fstream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <filesystem>

LocationConfig* Dispatcher::findLocation(const std::string& path, ServerConfig* sc) const
{
	LocationConfig*	lc = nullptr;
	size_t			longest_match = 0;

	for (auto& it : sc->_locations)
	{
		const std::string&	cur_location = it.first;
		size_t				cur_location_len = cur_location.length();
		// std::cout << "search for: '" << cur_location << "' in '" << path << "'\n";

		if (path.rfind(cur_location, 0) != 0)
			continue;

		if (path.length() > cur_location_len && path[cur_location_len] != '/')
			continue;

		if (cur_location_len > longest_match)
		{
			longest_match = cur_location_len;
			lc = &it.second;
			// std::cout << "'" << cur_location << "' ist neue location\n";
		}
	}
	return (lc);
}

void Dispatcher::checkMethodAllowed(std::string method, std::vector<std::string> allowed_methods) const
{
	auto it = std::find(allowed_methods.begin(), allowed_methods.end(), method);

	if (it == allowed_methods.end())
		throw HttpException(405);
}

std::string Dispatcher::cwd() const
{
	return (std::filesystem::current_path().string());
}

std::string Dispatcher::getDefaultBody(int code) const
{
	std::string body;

	std::string full_path = cwd() + "/default_pages/" + std::to_string(code) + ".html";
	try
	{
		body = readFile(full_path);
	}
	catch(const std::exception& e)
	{
		body = "<!DOCTYPE html>\n"
				"<html>\n"
				"<body>\n"
				"    <h1>" + std::to_string(code) + " - " + getStatusText(code) + "</h1>\n"
				"</body>\n"
				"</html>\n";
	}
	return (body);
}

std::string Dispatcher::readFile(std::string& filepath) const
{
	// get file mode and size
	struct stat statbuf;
	if (stat(filepath.c_str(), &statbuf) == -1)
		throw std::runtime_error("stat failed");
	if (!S_ISREG(statbuf.st_mode))
		throw std::runtime_error("given file is not a regular file");

	// open file
	int fd = open(filepath.c_str(), O_RDONLY);
	if (fd == -1)
		throw std::runtime_error("open failed");

	// read file content
	std::string	file_content;
	file_content.resize(statbuf.st_size);
	ssize_t		total_read = 0;
	while (total_read < statbuf.st_size)
	{
		ssize_t bytes_read = read(fd, file_content.data() + total_read, statbuf.st_size - total_read);
		if (bytes_read == -1)
		{
			close(fd);
			throw std::runtime_error("read failed");
		}
		if (bytes_read == 0)
			break ;
		total_read += bytes_read;
	}
	close(fd);
	return (file_content);
}

HttpResponse Dispatcher::buildErrorResponse(int code, ServerConfig* sc, ConnectionMode cm)
{
	std::string body;

	try
	{
		if (sc == nullptr)
			throw std::runtime_error("no server config selected");

		if (sc->_error_pages.find(code) == sc->_error_pages.end())
			throw std::runtime_error("no error_page found for code " + std::to_string(code));
		
		std::string full_path = cwd() + sc->_error_pages[code];
		body = readFile(full_path);
	}
	catch(const std::exception& e)
	{
		body = getDefaultBody(code);
	}

	HttpResponse r;
	r._version = "HTTP/1.1";
	r._status_code = code;
	r._status_text = getStatusText(code);
	r._headers["Content-Type"] = "text/html";
	r._headers["Content-Length"] = std::to_string(body.length());
	if (cm == CON_KEEP_ALIVE)
		r._headers["Connection"] = "keep-alive";
	else
		r._headers["Connection"] = "close";
	r._body = body;

	return (r);
}

HttpResponse Dispatcher::handleRedirect(LocationConfig* lc)
{
	HttpResponse r;
	int redirect_code = lc->_redirect_code.value();

	r._version = "HTTP/1.1";

	r._status_code = redirect_code;
	r._status_text = getStatusText(redirect_code);

	if (redirect_code >= 300 && redirect_code < 400)
	{
		r._headers["Content-Length"] = "0";
		r._headers["Location"] = lc->_redirect_url.value();
	}
	else if (lc->_redirect_url.has_value())
	{
		r._headers["Content-Type"] = "text/plain";
		r._headers["Content-Length"] = std::to_string(lc->_redirect_url.value().length());
		r._headers["Connection"] = "keep-alive";
		r._body = lc->_redirect_url.value();
	}
	else
	{
		r._body = getDefaultBody(redirect_code);
		// std::cout << r._body << std::endl;
		r._headers["Content-Type"] = "text/html";
		r._headers["Connection"] = "keep-alive";
		r._headers["Content-Length"] = std::to_string(r._body.length());
	}

	return (r);
}

std::string Dispatcher::getFullRootPath(LocationConfig* lc) const
{
	std::string path;
	if (lc->_root[0] == '/')	// relativ to workdir
	{
		path = lc->_root;
	}
	else						// relativ to executable
	{
		path = cwd() + "/" + lc->_root;
	}
	return (path);
}

HttpResponse Dispatcher::handleStatic(const HttpRequest &request, LocationConfig* lc)
{
	HttpResponse r;

	if (lc == nullptr)
		throw HttpException(500);

	std::string full_path = getFullRootPath(lc) + request._path;
	// std::cout << full_path << std::endl;

	// pfad vorhanden?
	struct stat statbuf;
	if (stat(full_path.c_str(), &statbuf) == -1)
		throw HttpException(404);

	std::string body;
	if (S_ISREG(statbuf.st_mode)) // path is a file
	{
		try
		{
			body = readFile(full_path);
		}
		catch(const std::exception& e)
		{
			throw HttpException(404);
		}
	}
	else if (S_ISDIR(statbuf.st_mode)) // path is a directory
	{
		if (lc->_index.length() > 0) // index file exist
		{
			std::string index_path = full_path + "/" + lc->_index;

			try
			{
				body = readFile(index_path);
			}
			catch(const std::exception& e)
			{
				if (lc->_autoindex == true)
					body = "TODO autoindex\n";
				else
					throw HttpException(404);
			}
		}
		else // no index file
		{
			if (lc->_autoindex == true)
				body = "TODO autoindex\n";
			else
				throw HttpException(403);
		}
	}
	else
	{
		throw HttpException(502);
	}

	// TODO mime type festlegen


	r._version = "HTTP/1.1";
	r._status_code = 200;
	r._status_text = getStatusText(200);
	r._body = body;
	r._headers["Connection"] = "keep-alive";
	r._headers["Content-Length"] = std::to_string(r._body.length());

	return (r);
}

HttpResponse Dispatcher::dispatch(const HttpRequest& request, ServerConfig* sc)
{
	try
	{
		// find location with longest path match
		LocationConfig*	lc = this->findLocation(request._path, sc);
		if (lc == nullptr)
			throw HttpException(404);

		if (lc->_redirect_code.has_value())
			return (handleRedirect(lc));

		checkMethodAllowed(request._method, lc->_methods);

		return (handleStatic(request, lc));
	}
	catch(const HttpException& e)
	{
		return(buildErrorResponse(e.code(), sc, CON_KEEP_ALIVE));
	}

	HttpResponse dummy;
	return dummy;
}
