#include "Dispatcher.hpp"
#include "HttpException.hpp"
#include "HttpStatus.hpp"
#include "HttpMimeType.hpp"

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

std::string	Dispatcher::getConnectionMode(const std::map<std::string, std::string>& headers) const
{
	auto it = headers.find("connection");
	if (it != headers.end() && it->second == "close")
		return ("close");
	return ("keep-alive");
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
		body = getDefaultBody(code);
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

HttpResponse Dispatcher::handleRedirect(LocationConfig* lc, const HttpRequest& request)
{
	HttpResponse r;
	int redirect_code = lc->_redirect_code.value();

	r._version = "HTTP/1.1";
	r._status_code = redirect_code;
	r._status_text = getStatusText(redirect_code);
	r._headers["Connection"] = getConnectionMode(request._headers);

	if (redirect_code >= 300 && redirect_code < 400)
	{
		r._headers["Content-Length"] = "0";
		r._headers["Location"] = lc->_redirect_url.value();
	}
	else if (lc->_redirect_url.has_value())
	{
		r._headers["Content-Type"] = "text/plain";
		r._headers["Content-Length"] = std::to_string(lc->_redirect_url.value().length());
		r._body = lc->_redirect_url.value();
	}
	else
	{
		r._body = getDefaultBody(redirect_code);
		r._headers["Content-Type"] = "text/html";
		r._headers["Content-Length"] = std::to_string(r._body.length());
	}

	return (r);
}

std::string Dispatcher::getFullRootPath(LocationConfig* lc) const
{
	std::string path;
	if (lc->_root[0] == '/')	// relativ to workdir -> forbidden? oder wird gehandelt vom executable ausgehend?
	{
		path = lc->_root;

	}
	else if (lc->_root[0] == '.' && lc->_root[1] == '/') // relativ to executable 
	{
		std::string rootWithoutDot = lc->_root;
		rootWithoutDot= rootWithoutDot.erase(0,1);
		path = cwd() + rootWithoutDot;
	}
	else						// relativ to executable
	{
		path = cwd() + "/" + lc->_root;
	}
	return (path);
}

std::string Dispatcher::getFullUploadPath(LocationConfig* lc, std::string rootPath)
{
	std::string uploadDir;
	uploadDir= lc->_upload_store.value();
	
	if (uploadDir.find("./") == 0 )
	{
		uploadDir.erase(0,1);
		rootPath += uploadDir; 
	}
	else if(uploadDir.find('/') == 0)						// absolut to executable
	{
		rootPath += uploadDir;
	}
	else
	{
		rootPath += '/' + uploadDir;
	}
	return (rootPath);
}

HttpResponse Dispatcher::handleStatic(const HttpRequest &request, LocationConfig* lc)
{
	HttpResponse r;

	if (lc == nullptr)
		throw HttpException(500);

	std::string full_path = getFullRootPath(lc) + request._path;

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
			r._headers["Content-Type"] = getMimeTypeFromFile(full_path);
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
				r._headers["Content-Type"] = getMimeTypeFromFile(index_path);
			}
			catch(const std::exception& e)
			{
				if (lc->_autoindex == true)
				{
					body = "TODO autoindex\n";
					r._headers["Content-Type"] = "text/html";
				}
				else
					throw HttpException(404);
			}
		}
		else // no index file
		{
			if (lc->_autoindex == true)
			{
				body = "TODO autoindex\n";
				r._headers["Content-Type"] = "text/html";
			}
			else
				throw HttpException(403);
		}
	}
	else
	{
		throw HttpException(502);
	}

	r._version = "HTTP/1.1";
	r._status_code = 200;
	r._status_text = getStatusText(200);
	r._body = body;
	r._headers["Connection"] = getConnectionMode(request._headers);
	r._headers["Content-Length"] = std::to_string(r._body.length());

	return (r);
}


std::string Dispatcher::buildFileName(const HttpRequest& request)
{
	std::string filename;
	// handle decoded and encoded URLs 
	// firs URL decode and regex check 
	std::regex traversalPattern("(^|/)\\.\\.(/|$)"); // 
	
	/*
		(^|/)    start of string OR slash
	\\.\\.   exactly two dots
	(/|$)    slash OR end of string

	So it matches:

	../x
	/a/../b
	/a/..
	..
	*/
	
	size_t filenameStart = request._path.find_last_of('/');
	if (filenameStart == std::string::npos )
		throw HttpException(500);
	filename = request._path.substr(filenameStart + 1);
	return filename;
}
void Dispatcher::createDirAndFile(const HttpRequest& request, std::string uploadpath, std::string target)
{
	std::filesystem::create_directories(uploadpath);
	std::ofstream NewFile(target, std::ios::binary);
	//std::ofstream NewFile(target);
	if (!NewFile.is_open())
		throw HttpException(500);
	NewFile.write(request._body.data(), request._body.size());
	//NewFile << request._body;
	if(!NewFile)
		throw HttpException(500);
	NewFile.close();
}

bool Dispatcher::fileExists(const std::string& target) const
{
	struct stat statbuf;
	if (stat(target.c_str(), &statbuf) == -1)
		return false;

	return S_ISREG(statbuf.st_mode); // chekc regularfile

}

HttpResponse Dispatcher::handleUpload(const HttpRequest& request, LocationConfig* lc)
{
	std::string uploadpath;
	bool fileExisted;
	std::cout<< "rootPath:  " << lc->_root << std::endl;

	std::string rootPath = getFullRootPath(lc); // anhaegen 
	std::cout<< "rootPath to danceserv:  " << rootPath << std::endl;
	if (lc->_upload_store.has_value())
		uploadpath = getFullUploadPath(lc, rootPath);
	else
	{
		std::cout << "now upload path" << std::endl; // or value_or
		throw HttpException(404);
	}
	std::string filename = buildFileName(request);
	
	std::cout << "requst path: " << request._path << std::endl;
	std::cout<< "filename: " <<filename << std::endl;
	std::cout << "uploadPath: " << uploadpath<< std::endl;

	
	std::string target = uploadpath + "/" + filename;

	fileExisted = fileExists(target);
	createDirAndFile(request, uploadpath,target);

	std::cout << "body size: " << request._body.size() << std::endl;
	std::cout << "body content: " << request._body << std::endl;
	
	HttpResponse respond;
	respond._version = "HTTP/1.1";
	if (fileExisted == true)
	{
		respond._status_code = 200;
		respond._status_text = "OK"; // overwrites file 204 No Content auch moeglich
	}
	else
	{
		respond._status_code = 201;
		respond._status_text = "Created";
	}
	// differ body content 
	respond._headers["Content-Length"] = "0";
	respond._headers["Connection"] = getConnectionMode(request._headers);
	
	return (respond);
}



HttpResponse Dispatcher::handleCgi(const HttpRequest& request, LocationConfig* lc)
{
	// TODO gha

	(void) request;
	(void) lc;
	HttpResponse dummy;
	return (dummy);
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
		// reject path traversal .. or /../

		if (lc->_cgi_map.size() > 0)
			return (handleCgi(request, lc));
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
