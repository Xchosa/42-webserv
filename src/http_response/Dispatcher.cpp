#include "Dispatcher.hpp"
#include "HttpException.hpp"
#include "HttpStatus.hpp"
#include "HttpMimeType.hpp"

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

std::string	Dispatcher::getConnectionMode(const std::map<std::string, std::string>& headers) const
{
	auto it = headers.find("connection");
	if (it != headers.end() && it->second == "close")
		return ("close");
	return ("keep-alive");
}

std::string Dispatcher::getDefaultErrorBody(int code) const
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

HttpResponse Dispatcher::buildErrorResponse(int code, ServerConfig* sc, ConnectionMode cm, const HttpRequest& request)
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
		body = getDefaultErrorBody(code);
	}

	HttpResponse r;
	r._version = "HTTP/1.1";
	r._status_code = code;
	r._status_text = getStatusText(code);
	r._headers["Content-Type"] = "text/html";
	r._headers["Content-Length"] = std::to_string(body.length());
	if (cm == CON_KEEP_ALIVE)
		r._headers["Connection"] = getConnectionMode(request._headers);
	else
		r._headers["Connection"] = "close";
	r._body = body;

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
		//std::string executable = std::filesystem::path(lc->_root).lexically_normal();
		//path = cwd() + "/" + executable;
		path = cwd() + "/" + lc->_root;
	}
	//std::string normalizedPath = resolvePath(path);
	return (path);
}

// spaeter in Dispatcher.cpp
void Dispatcher::isWithin(const std::string& base_path, std::string& user_path)
{
	//std::cout << "[DEBUG] base_path: " << base_path<< std::endl;
	//std::cout << "[DEBUG] request_pth: " << user_path<< std::endl;


	std::filesystem::path fs_base_path = std::filesystem::absolute(base_path).lexically_normal();
	std::filesystem::path fs_user_path = std::filesystem::absolute(user_path).lexically_normal();

	
	user_path = fs_user_path.string();
	std::string base_path_norm = fs_base_path.string() + "/" ; 

	size_t n = user_path.find(base_path_norm);
	if (n != 0 || n == std::string::npos)
		throw HttpException(403);

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
			return (handleRedirect(lc, request));
		
		checkMethodAllowed(request._method, lc->_methods);

		if (lc->_cgi_map.size() > 0)
			return (handleCgi(request, sc, lc));
		else if (lc->_upload_store.has_value())
		{
			std::cout << lc->_upload_store.value() << std::endl;
			return (handleUpload(request, lc));
		}
		else
			return(handleStatic(request, lc));
	}
	catch(const HttpException& e)
	{
		return(buildErrorResponse(e.code(), sc, CON_KEEP_ALIVE, request));
	}

	HttpResponse dummy;
	return dummy;
}
