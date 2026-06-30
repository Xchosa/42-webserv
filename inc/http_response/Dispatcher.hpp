#pragma once

#include <iostream>
#include <string>
#include <regex>
#include <algorithm>
#include <fstream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <filesystem>
#include <vector>
//#include <cassert>

#include "ServerConfig.hpp"
#include "LocationConfig.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "HttpException.hpp"
#include "HttpStatus.hpp"
#include "HttpMimeType.hpp"
#include "CgiSession.hpp"

enum ConnectionMode
{
	CON_CLOSE,
	CON_KEEP_ALIVE
};

enum DispatchResult
{
	DP_DONE,
	DP_CGI_PENIDNG
};

class Dispatcher
{
	private:
		// handler (baut response)
		HttpResponse handleRedirect(LocationConfig* lc, const HttpRequest& request);
		HttpResponse handleStatic(const HttpRequest& request, LocationConfig* lc);
		HttpResponse handleUpload(const HttpRequest& request, LocationConfig* lc);
		CgiSession	 handleCgi(const HttpRequest& request, ServerConfig* sc, LocationConfig* lc);

		// helper
		LocationConfig*	findLocation(const std::string& path, ServerConfig* sc) const;
		void			checkMethodAllowed(std::string method, std::vector<std::string> allowed_methods) const;
		std::string		readFile(std::string& filepath) const;
		std::string		cwd() const;
		std::string		getDefaultErrorBody(int code) const;
		std::string		getFullRootPath(LocationConfig* lc) const;
		void			isWithin(const std::string& base_path, std::string& user_path);

		// handle upload
		std::string		getFullUploadPath(LocationConfig* lc, std::string rootPath);
		std::string 	buildFileName(std::string user_path);
		bool			createDirAndFile(const HttpRequest& request, std::string uploadpath);
		bool			fileExists(const std::string& target) const;
		

		// handle cgi
		std::string		upperString(std::string str) const;
		void			checkForCgi(const HttpRequest& request, std::string& interpreter, std::string& path, std::vector<std::string>& env, LocationConfig* lc);
		void			buildEnv(std::vector<std::string>& env, const HttpRequest& request, std::string& path, std::string& script_path, ServerConfig* sc);
		CgiSession		startCgi(const HttpRequest& request, ServerConfig* sc, LocationConfig* lc);

		//handle autoindex
		std::string		autoIndexBody(const std::string& dir_path, const std::string& request_path);

	public:
		// OCF
		Dispatcher() = default;
		Dispatcher(const Dispatcher& other) = default;
		Dispatcher& operator=(const Dispatcher& other) = default;
		~Dispatcher() = default;

		// member functions
		DispatchResult	dispatch(const HttpRequest& request, ServerConfig* sc, HttpResponse& response_out, CgiSession& cgi_out); // sucht richtigen handler und passende location_config
		HttpResponse 	buildErrorResponse(int code, ServerConfig* sc, ConnectionMode cm, const HttpRequest& request);
		std::string		getConnectionMode(const std::map<std::string, std::string>& headers) const;
		HttpResponse	parseCgiOutput(std::string& output);
};


std::string resolvePath(std::string new_path);