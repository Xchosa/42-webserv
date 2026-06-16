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

HttpResponse Dispatcher::buildErrorResponse(int code, ServerConfig* sc)
{
	std::string body;

	try
	{
		if (sc->_error_pages.find(code) == sc->_error_pages.end())
			throw std::runtime_error("no error_page found for code " + std::to_string(code));
		
		std::string full_path = cwd() + sc->_error_pages[code];
		body = readFile(full_path);
	}
	catch(const std::exception& e)
	{
		std::string full_path = cwd() + "/errors/" + std::to_string(code) + ".html";
		try
		{
			body = readFile(full_path);
		}
		catch(const std::exception& e)
		{
			body = "<!DOCTYPE html><html><body><h1>" + std::to_string(code) + " - " + getStatusText(code) + "</h1></body></html>";
		}
	}

	HttpResponse r;
	r._version = "HTTP/1.1";
	r._status_code = code;
	r._status_text = getStatusText(code);
	r._headers["Content-Length"] = std::to_string(body.length());
	r._body = body;

	return (r);
}

HttpResponse Dispatcher::handleRedirect(LocationConfig* lc)
{
	HttpResponse r;

	// TODO: text ist aktuell nur fuer code 301

	r._version = "HTTP/1.1";
	r._status_code = lc->_redirect_code.value();
	r._status_text = "Moved Permanently";
	r._headers["Location"] = lc->_redirect_url.value();
	r._headers["Content-Length"] = "0";

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


		// find correct handler
		// ...

		// find correct location
		// ...
	}
	catch(const HttpException& e)
	{
		return(buildErrorResponse(e.code(), sc));
	}

	HttpResponse dummy;
	return dummy;
}
