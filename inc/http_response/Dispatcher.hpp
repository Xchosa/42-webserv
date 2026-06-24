#pragma once

#include <iostream>
#include <string>
#include <regex>
#include <algorithm>
#include <fstream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <filesystem>
#include <vector>

#include "ServerConfig.hpp"
#include "LocationConfig.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "HttpException.hpp"
#include "HttpStatus.hpp"
#include "HttpMimeType.hpp"

enum ConnectionMode
{
	CON_CLOSE,
	CON_KEEP_ALIVE
};

class Dispatcher
{
	private:
		// handler (baut response)
		HttpResponse handleRedirect(LocationConfig* lc, const HttpRequest& request);
		HttpResponse handleStatic(const HttpRequest& request, LocationConfig* lc);
		HttpResponse handleCgi(const HttpRequest& request, ServerConfig* sc, LocationConfig* lc);
		HttpResponse handleUpload(const HttpRequest& request, LocationConfig* lc);

		// helper
		LocationConfig*	findLocation(const std::string& path, ServerConfig* sc) const;
		void			checkMethodAllowed(std::string method, std::vector<std::string> allowed_methods) const;
		std::string		readFile(std::string& filepath) const;
		std::string		cwd() const;
		std::string		getDefaultErrorBody(int code) const;
		std::string		getFullRootPath(LocationConfig* lc) const;
		std::string		getConnectionMode(const std::map<std::string, std::string>& headers) const;

		// handle upload
		std::string		getFullUploadPath(LocationConfig* lc, std::string rootPath);
		std::string		buildFileName(const HttpRequest& request);
		void			createDirAndFile(const HttpRequest& request, std::string uploadpath, std::string target);
		bool			fileExists(const std::string& target) const;

		// handle cgi
		std::string		upperString(std::string str) const;

	public:
		// OCF
		Dispatcher() = default;
		Dispatcher(const Dispatcher& other) = default;
		Dispatcher& operator=(const Dispatcher& other) = default;
		~Dispatcher() = default;

		// member functions
		HttpResponse	dispatch(const HttpRequest& request, ServerConfig* sc); // sucht richtigen handler und passende location_config
		HttpResponse 	buildErrorResponse(int code, ServerConfig* sc, ConnectionMode cm, const HttpRequest& request);
};

